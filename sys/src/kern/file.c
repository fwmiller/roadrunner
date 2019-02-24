/* 
 *  Roadrunner/pk
 *    Copyright (C) 1989-2001  Cornfed Systems, Inc.
 *
 *  The Roadrunner/pk operating system is free software; you can
 *  redistribute and/or modify it under the terms of the GNU General
 *  Public License, version 2, as published by the Free Software
 *  Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this program; if not, write to the Free
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA 02111-1307 USA
 *
 *  More information about the Roadrunner/pk operating system of
 *  which this file is a part is available on the World-Wide Web
 *  at: http://www.cornfed.com.
 *
 */

#include <errno.h>
#include <fs.h>
#if _DEBUG
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <sys.h>
#include <sys/ioctl.h>
#include <unistd.h>

char *filetabpaths = NULL;
struct file filetab[FILES];
struct mutex filetabmutex;

void
file_clear(file_t file)
{
    file->type = FT_REGULAR;
    bzero(file->path, PATH_LENGTH);
    file->refcnt = 0;
    file->fs = NULL;
    file->flags = 0;
    file->filesize = 0;
    file->pos = 0;
    file->data = NULL;
    file->bufsize = 0;
    file->buf = NULL;
}

int
file_attr(file_t file, attrlist_t attr)
{
    fs_t fs = (fs_t) file->fs;

    if (file == NULL || file->type != FT_REGULAR)
	return EINVAL;

    if (fs->fsops->attr != NULL)
	return fs->fsops->attr(file, attr);
    return ENOSYS;
}

int
file_close(file_t file)
{
    int result = 0;

    if (file->refcnt > 1) {
	file->refcnt--;
	return 0;
    }
    if (file->fs->fsops->close != NULL)
	result = file->fs->fsops->close(file);

    mutex_lock(&filetabmutex);
    file_clear(file);
    mutex_unlock(&filetabmutex);
    return result;
}

int
file_ioctl(file_t file, int cmd, void *args)
{
    fs_t fs = file->fs;

    if (file == NULL || file->type != FT_REGULAR)
	return EINVAL;

    if (cmd == GET_FILE_SIZE) {
	if (args == NULL)
	    return EINVAL;
	*((u_long *) args) = file->filesize;
	return 0;
    } else if (cmd == GET_FILE_POS) {
	if (args == NULL)
	    return EINVAL;
	*((u_long *) args) = file->pos;
	return 0;
    }
    if (fs->fsops->ioctl != NULL)
	return fs->fsops->ioctl(file, cmd, args);
    return ENOSYS;
}

int
file_open(char *path, int flags, file_t * file)
{
    fs_t fs;
    char *fullpath, *srchpath;
    int fileno, fspathlen, result;

    /* Make sure we have an absolute path */
    if (path[0] == '/')
	fullpath = path;
    else {
	char *cwd;

	if ((cwd = (char *) malloc(PATH_LENGTH)) == NULL)
	    return ENOMEM;
	if ((fullpath = (char *) malloc(PATH_LENGTH)) == NULL) {
	    free(cwd);
	    return ENOMEM;
	}
	proc_getcwd(cwd, PATH_LENGTH);
	mkpath(cwd, path, fullpath);
	free(cwd);
    }

    fs = fs_lookup(fullpath);
    if (fs == NULL) {
#if _DEBUG
	kprintf("file_open: file system not found\n");
#endif
	if (fullpath != path)
	    free(fullpath);
	return ENOENT;
    }

    /*
     * Check whether file is already open
     */
    srchpath = (char *) malloc(PATH_LENGTH);
    if (srchpath == NULL) {
	if (fullpath != path)
	    free(fullpath);
	return ENOMEM;
    }
    mutex_lock(&filetabmutex);

    for (fileno = 0; fileno < FILES; fileno++) {
	*file = &(filetab[fileno]);
	if ((*file)->fs == NULL)
	    continue;
	mkpath((*file)->fs->path, (*file)->path, srchpath);
	if (strcmp(fullpath, srchpath) == 0) {
	    free(srchpath);
	    if (fullpath != path)
		free(fullpath);
	    (*file)->refcnt++;
	    mutex_unlock(&filetabmutex);
	    return 0;
	}
    }
    free(srchpath);

    /*
     * Allocate a file table entry
     */
    for (fileno = 0; fileno < FILES && filetab[fileno].fs != NULL;
	 fileno++);
    if (fileno == FILES) {
#if _DEBUG
	kprintf("file_open: file table full\n");
#endif
	if (fullpath != path)
	    free(fullpath);
	mutex_unlock(&filetabmutex);
	return EAGAIN;
    }
    *file = &(filetab[fileno]);
    (*file)->fs = fs;
    mutex_unlock(&filetabmutex);

    (*file)->type = FT_REGULAR;
    (*file)->refcnt = 1;
    (*file)->flags = flags;
    (*file)->filesize = 0;
    (*file)->pos = 0;
    (*file)->buf = NULL;

    fspathlen = strlen(fs->path);
    if (*(fullpath + fspathlen) != '/')
	strcpy((*file)->path, "/");
    strcat((*file)->path, fullpath + fspathlen);

    /* Specific file system open routine */
    if (fs->fsops->open == NULL) {
	file_clear(*file);
	if (fullpath != path)
	    free(fullpath);
	return ENOSYS;
    }
    if ((result = fs->fsops->open(*file)) < 0) {
#if _DEBUG
	kprintf("file_open: open %s failed (%s)\n",
		fullpath, strerror(result));
#endif
	file_clear(*file);
    }
    if (fullpath != path)
	free(fullpath);
    return result;
}

int
file_read(file_t file, char *buf, int *len)
{
    fs_t fs = file->fs;
    int m, n, nleft, result;

    if (fs->fsops->read == NULL) {
#if _DEBUG
	kprintf("file_read: no specific read routine\n");
#endif
	return ENOSYS;
    }
    for (n = 0, nleft = (int) *len;
	 !(file->flags & (F_EOF | F_ERR)) && nleft > 0;) {

	/* Make sure we have a buffer to read data from */
	if (file->buf == NULL) {
	    file->buf = bget(file->bufsize);
	    blen(file->buf) = file->bufsize;

	    if ((result = fs->fsops->read(file)) < 0) {
#if _DEBUG
		kprintf("file_read: specific read failed (%s)\n",
			strerror(result));
#endif
		file->flags |= F_ERR;
		*len = n;
		return result;
	    }
	    bpos(file->buf) = 0;
	}
	m = blen(file->buf) - bpos(file->buf);

	/* Check for user buffer overflow */
	if (m > nleft)
	    m = nleft;

	/* Check for end-of-file */
	if (file->filesize < MAX_FILE_SIZE
	    && file->pos + m >= file->filesize) {

	    m = file->filesize - file->pos;
	    file->flags |= F_EOF;
	}
	/* Copy data to user buffer */
	bcopy(bstart(file->buf) + bpos(file->buf), buf + n, m);

	/* Update counters */
	file->pos += m;
	n += m;
	nleft -= m;

	/* Discard buffer if data is exhausted */
	if ((bpos(file->buf) += m) == blen(file->buf)) {
	    brel(file->buf);
	    file->buf = NULL;
	}
    }
    *len = n;
    return 0;
}

int
file_readdir(file_t file, char *entry)
{
    fs_t fs = (fs_t) file->fs;

    if (file == NULL || file->type != FT_REGULAR)
	return EINVAL;

    if (fs->fsops->readdir != NULL)
	return fs->fsops->readdir(file, entry);
    return ENOSYS;
}

int
file_unlink(char *path)
{
    fs_t fs;
    char *fullpath, *srchpath;
    file_t file;
    int fileno;

    /* Make sure we have an absolute path */
    if (path[0] == '/')
	fullpath = path;
    else {
	char *cwd;

	if ((cwd = (char *) malloc(PATH_LENGTH)) == NULL)
	    return ENOMEM;
	if ((fullpath = (char *) malloc(PATH_LENGTH)) == NULL) {
	    free(cwd);
	    return ENOMEM;
	}
	proc_getcwd(cwd, PATH_LENGTH);
	mkpath(cwd, path, fullpath);
	free(cwd);
    }

    fs = fs_lookup(fullpath);
    if (fs == NULL) {
#if _DEBUG
	kprintf("file_unlink: file system not found\n");
#endif
	if (fullpath != path)
	    free(fullpath);
	return ENOENT;
    }

    /* Check whether file is already open */
    srchpath = (char *) malloc(PATH_LENGTH);
    if (srchpath == NULL) {
	if (fullpath != path)
	    free(fullpath);
	return ENOMEM;
    }
    mutex_lock(&filetabmutex);

    for (fileno = 0; fileno < FILES; fileno++) {
	file = &(filetab[fileno]);
	if (file->fs == NULL)
	    continue;
	mkpath(file->fs->path, file->path, srchpath);
	if (strcmp(fullpath, srchpath) == 0) {
	    free(srchpath);
	    if (fullpath != path)
		free(fullpath);
	    mutex_unlock(&filetabmutex);
	    return EEXIST;
	}
    }
    mutex_unlock(&filetabmutex);
    free(srchpath);

    if (fs->fsops->unlink != NULL)
	return fs->fsops->unlink(path);
    return ENOSYS;
}

int
file_write(file_t file, char *buf, int *len)
{
    fs_t fs = file->fs;
    int bufleft, n, nleft, nwritten, result;

    if (fs->fsops->write == NULL) {
#if _DEBUG
	kprintf("file_write: no specific write routine\n");
#endif
	return ENOSYS;
    }
    for (nwritten = 0, nleft = *len; nleft > 0;) {
	if (file->buf != NULL)
	    bufleft = blen(file->buf) - bpos(file->buf);
	else {
	    file->buf = bget(file->bufsize);
	    blen(file->buf) = file->bufsize;
	    bpos(file->buf) = 0;
	    bufleft = blen(file->buf);
	}
	if ((n = bufleft) > nleft)
	    n = nleft;

	bcopy(buf + nwritten, bstart(file->buf) + bpos(file->buf), n);
	file->pos += n;
	bpos(file->buf) += n;
	nwritten += n;
	nleft -= n;

	if ((bufleft -= n) == 0) {
	    result = fs->fsops->write(file);
	    if (result < 0) {
#if _DEBUG
		kprintf("file_write: specific write failed (%s)\n",
			strerror(result));
#endif
		*len = nwritten;
		return result;
	    }
	    if (file->buf != NULL) {
		brel(file->buf);
		file->buf = NULL;
	    }
	}
    }
    *len = nwritten;
    return 0;
}
