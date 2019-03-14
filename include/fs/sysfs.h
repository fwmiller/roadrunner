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

#ifndef __SYSFS_H
#define __SYSFS_H

#if _KERNEL

#include <fs.h>

#define SYSFS_FILE_ATTRIBUTES		1
#define SYSFS_FILE_KEY_ATTR		0	/* Name */

#define SYSFILE_NAME_LEN		16

#define SYSFILE_NAMES			7

#define SYSFILE_NAME_BLKS		0
#define SYSFILE_NAME_BUFS		1
#define SYSFILE_NAME_FILES		2
#define SYSFILE_NAME_FILESYSTEMS	3
#define SYSFILE_NAME_PROCS		4
#define SYSFILE_NAME_REGIONS		5
#define SYSFILE_NAME_VM			6

extern char *sysfs_filenames[];

int sysfs_init ();
int sysfs_shut ();
int sysfs_mount (fs_t fs);
int sysfs_unmount (fs_t fs);
int sysfile_open (file_t file);
int sysfile_close (file_t file);
int sysfile_ioctl (file_t file, int cmd, void *args);
int sysfile_read (file_t file);
int sysfile_write (file_t file);
int sysfile_attr (file_t file, attrlist_t attr);
int sysfile_readdir (file_t file, char *entry);
int sysfile_unlink (char *path);

#endif /* _KERNEL */

#endif
