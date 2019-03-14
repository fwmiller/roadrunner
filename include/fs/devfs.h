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

#ifndef __DEVFS_H
#define __DEVFS_H

#if _KERNEL

#include <fs.h>

#define DEVFS_FILE_ATTRIBUTES	3
#define DEVFS_FILE_KEY_ATTR	2	/* Name */

int devfs_init ();
int devfs_shut ();
int devfs_mount (fs_t fs);
int devfs_unmount (fs_t fs);
int devfile_open (file_t file);
int devfile_close (file_t file);
int devfile_ioctl (file_t file, int cmd, void *args);
int devfile_read (file_t file);
int devfile_write (file_t file);
int devfile_attr (file_t file, attrlist_t attr);
int devfile_readdir (file_t file, char *entry);
int devfile_unlink (char *path);

#endif /* _KERNEL */

#endif
