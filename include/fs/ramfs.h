#ifndef __RAMFS_H
#define __RAMFS_H

#if _KERNEL

#include <fs.h>

#define DENAME_LEN		80
#define RAM_FILES		16
#define RAMFILE_BUFSIZE		1

struct ramfs_direntry
{
  char name[DENAME_LEN];
  u_long size;
};

struct ramfile
{
#define RAMF_INUSE	0x01
  int flags;

  char name[DENAME_LEN];
  u_long offset;
  u_long size;
};

typedef struct ramfs_direntry *ramfs_direntry_t;
typedef struct ramfile *ramfile_t;

extern int ramfs_entries;
extern ramfs_direntry_t dir;
extern int ramfiles_pos;

extern unsigned char ___bin_ramfs[];

extern ramfile_t ramfiletab;
extern struct mutex ramfiletabmutex;

void ramfile_clear (ramfile_t rf);
void ramfs_file_table_dump (int entries);
int ramfs_lookup (file_t file, char *path);

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
