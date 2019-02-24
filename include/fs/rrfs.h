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

#ifndef __RRFS_H
#define __RRFS_H

#if _KERNEL

#include <fs.h>
#include <sys/buf.h>
#include <sys/config.h>
#include <sys/mutex.h>
#include <sys/types.h>

#define RR_FILE_SYSTEMS		4
#define RR_FILES		FILES
#define RRFS_FILE_ATTRIBUTES	4
#define RRFS_FILE_KEY_ATTR	3      /* Name */

#endif				/* _KERNEL */

#define JUMP_SIZE		4
#define BOOTSIG_SIZE		2
#define BOOTLDR_SIZE \
    (SECTOR_SIZE - (JUMP_SIZE + sizeof(struct rrfs_params) + BOOTSIG_SIZE))
#define FAT_CHAIN_END		0xffffffff

#if _KERNEL

#define FAT_BLK_ENTRIES		(SECTOR_SIZE / sizeof(u_long))
#define FAT_BLKNO(CLUST)	((CLUST) / FAT_BLK_ENTRIES)

#define DE_SIZE			64
#define DE_NAME_LENGTH		48
#define DE_TIME_LENGTH		3
#define DE_DATE_LENGTH		3

/* Directory entry types */
#define DE_UNUSED		0
#define DE_DELETED		0xff

/* Directory entry attributes */
#define DE_ATTR_DIR		0x01
#define DE_ATTR_READ		0x02
#define DE_ATTR_WRITE		0x04
#define DE_ATTR_EXEC		0x08

/* Directory entry data and time offsets */
#define DE_DATE_MON		0
#define DE_DATE_DAY		1
#define DE_DATE_YEAR		2
#define DE_TIME_HOUR		0
#define DE_TIME_MIN		1
#define DE_TIME_SEC		2

#define CLUST2BLKNO(RRFS, CLUST)					\
    ((RRFS)->mbr->params.bootsectors +					\
     (2 * (RRFS)->mbr->params.fatsectors) +				\
     ((CLUST) * (RRFS)->mbr->params.sectorsperclust))

#endif				/* _KERNEL */

struct rrfs_params {
    u_long tracks;
    u_long heads;
    u_long sectorspertrack;
    u_short bytespersector;
    u_long sectors;
    u_short bootsectors;
    u_long fatsectors;
    u_short sectorsperclust;
    u_long clusters;
} __attribute__ ((packed));

struct mbr {
    char jump[JUMP_SIZE];
    struct rrfs_params params;
    char boot[BOOTLDR_SIZE];
    char bootsig[BOOTSIG_SIZE];
} __attribute__ ((packed));

#if _KERNEL

struct rrfs {
#define RFS_FATBLK_DIRTY	0x01
    int flags;
    char *mbrbuf;
    struct mbr *mbr;
    u_long fatblkno;
    buf_t fatbuf;
    u_long *fatblk;
};

struct rrfile {

#define RF_INUSE		0x01
#define RF_ROOTDIR		0x02
#define RF_NEEDCLUST		0x04
    int flags;

    u_long declust;
    u_long deoff;
    u_long firstclust;
    u_long currclust;
};

struct direntry {
    char name[DE_NAME_LENGTH];
    u_short attr;
    u_char time[DE_TIME_LENGTH];
    u_char date[DE_DATE_LENGTH];
    u_long size;
    u_long start;
} __attribute__ ((packed));

#endif				/* _KERNEL */

typedef struct rrfs_params *rrfs_params_t;
typedef struct mbr *mbr_t;

#if _KERNEL

typedef struct rrfs *rrfs_t;
typedef struct rrfile *rrfile_t;
typedef struct direntry *direntry_t;

extern struct rrfs rrfstab[];
extern struct mutex rrfstabmutex;
extern struct rrfile rrfiletab[];
extern struct mutex rrfiletabmutex;

void rrfs_clear(rrfs_t rrfs);
void rrfs_nextpathelem(char *path, int *pos, char *name);
u_long rrfs_nextclust(fs_t fs, u_long clust);
u_long rrfs_clustalloc(fs_t fs);
u_long rrfs_clustappend(fs_t fs, u_long clust);

#if _DEBUG
void rrfs_direntrydump(direntry_t de);
#endif
int rrfs_readclust(file_t file, u_long clust, buf_t * b);
int rrfs_readmbr(fs_t fs);
int rrfs_readfatblk(fs_t fs, u_long clust);
int rrfs_writeclust(file_t file, u_long clust, buf_t * b);
int rrfs_writembr(fs_t fs);
int rrfs_writefatblk(fs_t fs);
int rrfs_lookup(file_t file, char *path);
int rrfs_updatedir(file_t file, u_long firstclust);
void rrfile_clear(rrfile_t rrfile);
int rrfile_create(file_t file, int directory);

int rrfs_init();
int rrfs_shut();
int rrfs_mount(fs_t fs);
int rrfs_unmount(fs_t fs);
int rrfile_open(file_t file);
int rrfile_close(file_t file);
int rrfile_ioctl(file_t file, int cmd, void *args);
int rrfile_read(file_t file);
int rrfile_write(file_t file);
int rrfile_attr(file_t file, attrlist_t attr);
int rrfile_readdir(file_t file, char *entry);
int rrfile_unlink(char *path);

#endif				/* _KERNEL */

#endif
