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

#ifndef __FS_H
#define __FS_H

#include <sys/buf.h>
#include <sys/config.h>
#include <sys/mutex.h>
#include <sys/types.h>

#define MAX_BLKNO		0xffffffff
#define MAX_FILE_SIZE		0xffffffff

#define FSREC_SIZE (sizeof(struct fsrec) + PATH_LENGTH + FS_NAME_LEN)

/* File types */
#define FT_REGULAR		0
#define FT_SOCKET		1

/* File flags */
#define F_ERR			0x04000000
#define F_EOF			0x02000000
#define F_DIR			0x01000000

/* File system attribute types */
#define ATTR_CHAR		0
#define ATTR_SHORT		1
#define ATTR_INT		2
#define ATTR_UCHAR		3
#define ATTR_USHORT		4
#define ATTR_UINT		5
#define ATTR_STRING		6

#define ATTR_NAME_LEN		32

#if _KERNEL

#define FSRECTAB_SIZE(CNT)  (sizeof(int) + (CNT) * FSREC_SIZE)

/* Forward declarations */
typedef struct fs *fs_t;
typedef struct file *file_t;

#endif				/* _KERNEL */

typedef struct fsrec *fsrec_t;
typedef struct fsrectab *fsrectab_t;
typedef struct attrlist *attrlist_t;

#if _KERNEL

/* File system operations vector */
typedef struct fsops {
    int slot;
    char name[FS_NAME_LEN];
    int (*init) ();
    int (*shut) ();
    int (*mount) (fs_t fs);
    int (*unmount) (fs_t fs);
    int (*open) (file_t file);
    int (*close) (file_t file);
    int (*ioctl) (file_t file, int cmd, void *args);
    int (*read) (file_t file);
    int (*write) (file_t file);
    int (*attr) (file_t file, attrlist_t attr);
    int (*readdir) (file_t file, char *entry);
    int (*unlink) (char *path);
} *fsops_t;

/* Mounted file system */
struct fs {
    int slot;
    char *path;			       /* File system mount point */
    fsops_t fsops;		       /* Operations for file system type */
    int devno;			       /* Attached device */
    u_long blkno;		       /* Current blkno on attached device */
    void *data;			       /* Specific file system data */
};

#endif				/* _KERNEL */

/*
 *
 * A table of file system information records is passed to a user when the
 * fs_getfstab() system call is made.  This table has the following
 * structure where all fields in each structure are contiguous:
 *
 *   fsrectab{}        fsrectab{}.recs             fsrec{}
 * +------------+   /+----------------*       /+-------------+
 * | entries    |  / |  struct fsrec  |      / | slot        |
 * +------------+ /  +----------------+     /  +-------------+
 * | recs       |/   |  struct fsrec  |    /+--| path        |
 * |            |    +----------------*   / |  +-------------+
 * +------------+    |        .       |  /  |  | fsops_name  |--+
 *               \            .         /   |  +-------------+  |
 *                \  |        .       |/    |  | devno       |  |
 *                 \ +----------------+     |  +-------------+  |
 *                  \|  struct fsrec  |     |  | blkno       |  |
 *                   +----------------+\    +->+-------------+  |
 *                                      \      | path string |  |
 *                                       \     |             |  |
 *                                        \    |             |  |
 *                                         \   +-------------+<-+
 *                                          \  | fsops_name  |
 *                                           \ | string      |
 *                                            \+-------------+
 *
 * XXX This system call and structures should all be replaced by reading
 * /proc/fs
 *
 */

/* File system information record */
struct fsrec {
    int slot;
    char *path;
    char *fsops_name;
    int devno;
    u_long blkno;
};

/* File system information record table */
struct fsrectab {
    int entries;
    char recs[1];
};

#if _KERNEL

/* Open file */
struct file {
    int slot;
    int type;			       /* Type (regular or socket) */
    char *path;			       /* Name (below mount point) */
    int refcnt;			       /* Descriptor reference count */
    fs_t fs;			       /* Resident file system */
    int flags;			       /* Descriptor flags */
    u_long filesize;		       /* Size in bytes */
    u_long pos;			       /* Current file position */
    void *data;			       /* Specific file system data */
    int bufsize;		       /* Buffer size for transfers */
    buf_t buf;			       /* Current buffer */
};

#endif				/* _KERNEL */

/* File system attribute */
typedef struct attr {
    u_char type;		       /* Attribute type */
    int len;			       /* Length of attribute type */
    char name[ATTR_NAME_LEN];	       /* Attribute name */
} *attr_t;

/* Attribute list */
struct attrlist {
    int n;			       /* Number of attribute in list */
    int key;			       /* Key attribute in list */
    attr_t *attr;		       /* List of attribute */
};

#if _KERNEL

extern struct fsops fsopstab[];
extern struct mutex fsopstabmutex;
extern struct fs fstab[];
extern struct mutex fstabmutex;
extern struct file filetab[];
extern struct mutex filetabmutex;

#endif

void getdir(char *path, char *dir);
void getname(char *path, char *name);
void mkpath(char *dir, char *name, char *path);

#if _KERNEL

void dumpfstab();
void dumpfiletab();
void fsops_clear(fsops_t fsops);
void fs_clear(fs_t fs);
void file_clear(file_t file);
void fstab_init();
fs_t fs_lookup(char *path);
int file_attr(file_t file, attrlist_t attr);
int file_close(file_t file);
int file_ioctl(file_t file, int cmd, void *args);
int file_open(char *path, int flags, file_t * file);
int file_read(file_t file, char *buf, int *len);
int file_readdir(file_t file, char *entry);
int file_write(file_t file, char *buf, int *len);
int file_unlink(char *path);
int fs_getfstab(fsrectab_t * fsrectab);
int fs_mount(fsops_t fsops, char *path, int devno, fs_t * fs);
int fs_unmount(fs_t fs);
int fsops_init(fsops_t fsops);
fsops_t fsops_inst(fsops_t fsops);
int fsops_shut(fsops_t fsops);
int fsops_uninst(fsops_t fsops);

#else

int getfstab(fsrectab_t * fsrectab);
int mount(const char *type, const char *path, const char *dev);
int unmount(char *path);
int attr(int fd, attrlist_t attr);
int readdir(int fd, char *entry);

#endif				/* _KERNEL */

#endif
