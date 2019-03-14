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
#include <sys/mem.h>

extern struct fsops fsopstab[FILE_SYSTEM_TYPES];
extern struct mutex fsopstabmutex;
extern char *filetabpaths;
extern struct file filetab[FILES];
extern struct mutex filetabmutex;

static char *fstabpaths = NULL;
struct fs fstab[FILE_SYSTEMS];
struct mutex fstabmutex;

void fsops_clear(fsops_t fsops)
{
	bzero(fsops->name, FS_NAME_LEN);
	fsops->init = NULL;
	fsops->shut = NULL;
	fsops->mount = NULL;
	fsops->unmount = NULL;
	fsops->open = NULL;
	fsops->close = NULL;
	fsops->ioctl = NULL;
	fsops->read = NULL;
	fsops->write = NULL;
	fsops->attr = NULL;
	fsops->readdir = NULL;
	fsops->unlink = NULL;
}

void fs_clear(fs_t fs)
{
	bzero(fs->path, PATH_LENGTH);
	fs->fsops = NULL;
	fs->devno = 0;
	fs->blkno = 0;
	fs->data = NULL;
}

void fstab_init()
{
	int i;

	for (i = 0; i < FILE_SYSTEM_TYPES; i++) {
		fsopstab[i].slot = i;
		fsops_clear(&(fsopstab[i]));
	}
	mutex_clear(&fsopstabmutex);

	fstabpaths = (char *)kmalloc(FILE_SYSTEMS * PATH_LENGTH);
	bzero(fstabpaths, FILE_SYSTEMS * PATH_LENGTH);

	for (i = 0; i < FILE_SYSTEMS; i++) {
		fstab[i].slot = i;
		fstab[i].path = fstabpaths + (i * PATH_LENGTH);
		fs_clear(&(fstab[i]));
	}
	mutex_clear(&fstabmutex);

	filetabpaths = (char *)kmalloc(FILES * PATH_LENGTH);
	bzero(filetabpaths, FILES * PATH_LENGTH);

	for (i = 0; i < FILES; i++) {
		filetab[i].slot = i;
		filetab[i].path = filetabpaths + (i * PATH_LENGTH);
		file_clear(&(filetab[i]));
	}
	mutex_clear(&filetabmutex);
}

fs_t fs_lookup(char *path)
{
	fs_t fs = NULL;

#define FS_PREFIX_LEN	64
	char prefix[FS_PREFIX_LEN];
	int fsno, len, pathlen, prefixlen = 0;

	/* Special case for root directory */
	if (strcmp(path, "/") == 0) {
		for (fsno = 0; fsno < FILE_SYSTEMS; fsno++) {
			fs = &(fstab[fsno]);
			if (strcmp(fs->path, "/") == 0)
				return fs;
		}
		return NULL;
	}
	mutex_lock(&fstabmutex);

	for (pathlen = strlen(path), fsno = 0; fsno < FILE_SYSTEMS; fsno++)
		if (fstab[fsno].fsops != NULL &&
		    pathlen >= (len = strlen(fstab[fsno].path))) {
			strncpy(prefix, path, len);
			prefix[len] = '\0';
			if (strcmp(prefix, fstab[fsno].path) == 0
			    && len > prefixlen) {
				fs = &(fstab[fsno]);
				prefixlen = len;
			}
		}
	mutex_unlock(&fstabmutex);
	return fs;
}

int fs_getfstab(fsrectab_t * fsrectab)
{
	fsrec_t fsrec;
	int fscnt, fsrectabsize, i;

	mutex_lock(&fstabmutex);

	/* Get mounted file system count */
	for (fscnt = 0, i = 0; i < FILE_SYSTEMS; i++)
		if (fstab[i].fsops != NULL)
			fscnt++;

	/* Build file system record table */
	fsrectabsize = FSRECTAB_SIZE(fscnt);
	*fsrectab = (fsrectab_t) malloc(fsrectabsize);
	if (*fsrectab == NULL) {
		mutex_unlock(&fstabmutex);
		return ENOMEM;
	}
	bzero(*fsrectab, fsrectabsize);

	(*fsrectab)->entries = fscnt;
	for (fscnt = 0, i = 0; i < FILE_SYSTEMS; fscnt++, i++)
		if (fstab[i].fsops != NULL) {
			fsrec = (fsrec_t) ((char *)&((*fsrectab)->recs) +
					   fscnt * FSREC_SIZE);
			fsrec->slot = i;
			fsrec->path = ((char *)fsrec) + sizeof(struct fsrec);

			strcpy(fsrec->path, fstab[i].path);
			fsrec->fsops_name = fsrec->path + PATH_LENGTH;
			strcpy(fsrec->fsops_name, fstab[i].fsops->name);
			fsrec->devno = fstab[i].devno;
			fsrec->blkno = fstab[i].blkno;
		}
	mutex_unlock(&fstabmutex);
	return 0;
}

int fs_mount(fsops_t fsops, char *path, int devno, fs_t * fs)
{
	int fsno, result;

	mutex_lock(&fstabmutex);

	/* Check for file system already mounted */
	for (fsno = 0; fsno < FILE_SYSTEMS; fsno++)
		if (fstab[fsno].fsops != NULL
		    && strcmp(fstab[fsno].path, path) == 0) {
			mutex_unlock(&fstabmutex);
			return EFSMOUNTED;
		}

	/* Allocate file system table entry */
	for (fsno = 0; fsno < FILE_SYSTEMS && fstab[fsno].fsops != NULL;
	     fsno++) ;
	if (fsno == FILE_SYSTEMS) {
		mutex_unlock(&fstabmutex);
		return EAGAIN;
	}
	/* Initialize file system table entry */
	*fs = &(fstab[fsno]);
	strcpy((*fs)->path, path);
	(*fs)->fsops = fsops;
	(*fs)->devno = devno;

	mutex_unlock(&fstabmutex);

	/* Execute specific file system mount if specified */
	if (fsops->mount == NULL) {
		mutex_lock(&fstabmutex);
		fs_clear(*fs);
		mutex_unlock(&fstabmutex);
		return ENOSYS;
	}
	if ((result = fsops->mount(*fs)) < 0) {
#if _DEBUG
		kprintf("fs_mount: specific mount failed (%s)\n",
			strerror(result));
#endif
		mutex_lock(&fstabmutex);
		fs_clear(*fs);
		mutex_unlock(&fstabmutex);
	}
	return result;
}

int fs_unmount(fs_t fs)
{
	file_t file;
	int i, result;

	mutex_lock(&filetabmutex);

	/*
	 * XXX Need to check whether any processes have a current working
	 * directory on the specified file system
	 */

	/* Check for any files that are open on the file system */
	for (i = 0; i < FILES; i++) {
		file = &(filetab[i]);
		if (file->fs == fs) {
#if _DEBUG
			kprintf("fs_unmount: file system in use\n");
#endif
			mutex_unlock(&filetabmutex);
			return EBUSY;
		}
	}
	mutex_unlock(&filetabmutex);

	/* Execute specific file system unmount if specified */
	if (fs->fsops->unmount != NULL) {
		result = fs->fsops->unmount(fs);
		if (result < 0) {
#if _DEBUG
			kprintf("fs_unmount: specific unmount failed (%s)\n",
				strerror(result));
#endif
			return result;
		}
	}
	/* Clear file system table entry */
	mutex_lock(&fstabmutex);
	fs_clear(fs);
	mutex_unlock(&fstabmutex);

	return 0;
}
