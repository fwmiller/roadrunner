#ifndef __RAMFS_H
#define __RAMFS_H

#if _KERNEL

#include <fs.h>

#define RAM_FILES	16

struct ramfile {
#define RAMF_INUSE	0x01
	int flags;

	u_long offset;
	u_long size;
};

typedef struct ramfile *ramfile_t;

void ramfile_clear(ramfile_t rf);

int ramfs_init ();
int ramfs_shut ();
int ramfs_mount (fs_t fs);
int ramfs_unmount (fs_t fs);
int ramfile_open (file_t file);
int ramfile_close (file_t file);
int ramfile_ioctl (file_t file, int cmd, void *args);
int ramfile_read (file_t file);
int ramfile_write (file_t file);
int ramfile_attr (file_t file, attrlist_t attr);
int ramfile_readdir (file_t file, char *entry);
int ramfile_unlink (char *path);

#endif /* _KERNEL */

#endif
