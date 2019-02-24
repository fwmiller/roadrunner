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
#include <fcntl.h>
#include <fs/sysfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mem.h>

void dump_blks(char *s);
void dump_bufs(char *s);
void dump_filetab(char *s);
void dump_fstab(char *s);
void dump_proctab(char *s);
void dump_regiontab(char *s);
void dump_vm(char *s, pt_t pd);

static int
sysfile_specific(file_t file, int sysfilenameno)
{
    char s[32];

    bzero(s, 32);
    strcpy(s, "/");
    strcat(s, sysfs_filenames[sysfilenameno]);

    if (strcmp(file->path, sysfs_filenames[sysfilenameno]) == 0 ||
	strcmp(file->path, s) == 0) {

	file->data = malloc(PAGE_SIZE);
	if (file->data == NULL) {
#if _DEBUG
	    kprintf("sysfile_specific: could not alloc file buffer\n");
#endif
	    return ENOMEM;
	}
	bzero(file->data, PAGE_SIZE);
	switch (sysfilenameno) {
	case SYSFILE_NAME_BLKS:
	    dump_blks(file->data);
	    break;
	case SYSFILE_NAME_BUFS:
	    dump_bufs(file->data);
	    break;
	case SYSFILE_NAME_FILES:
	    dump_filetab(file->data);
	    break;
	case SYSFILE_NAME_FILESYSTEMS:
	    dump_fstab(file->data);
	    break;
	case SYSFILE_NAME_PROCS:
	    dump_proctab(file->data);
	    break;
	case SYSFILE_NAME_REGIONS:
	    dump_regiontab(file->data);
	    break;
	case SYSFILE_NAME_VM:
	    dump_vm(file->data, (pt_t) current->context.tss->cr3);
	    break;
	//default:
	}
	file->filesize = strlen(file->data);
	file->bufsize = PAGE_SIZE;
#if _DEBUG
	kprintf("sysfile_specific: filesize %d\n", file->filesize);
#endif
	return 0;
    }
    return EFAIL;
}

int
sysfile_open(file_t file)
{
    int i;

    if (!(file->flags & O_RDONLY))
	return EINVAL;

    /* Special case for root directory */
    if (strcmp(file->path, "/") == 0 || strcmp(file->path, "") == 0) {
	file->data = (void *) 0;
	file->flags |= F_EOF | F_DIR;
	return 0;
    }

    for (i = 0; i < SYSFILE_NAMES; i++)
	if (sysfile_specific(file, i) == 0)
	    break;
    if (i == SYSFILE_NAMES) {
#if _DEBUG
	kprintf("sysfile_open: %s not found\n", file->path);
#endif
	return ENOENT;
    }
    return 0;
}
