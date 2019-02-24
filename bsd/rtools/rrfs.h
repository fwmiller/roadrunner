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

#include <inttypes.h>
#include <sys/types.h>

#define DEVICE			"/dev/fd0"
#define PAGE_SIZE		4096
#define PATH_LENGTH		PAGE_SIZE

#define SECTOR_SIZE		512
#define JUMP_SIZE		4
#define BOOTSIG_SIZE		2
#define BOOTLDR_SIZE \
    (SECTOR_SIZE - (JUMP_SIZE + sizeof(struct rrfs_params) + BOOTSIG_SIZE))

#define BOOT_SECTORS		16

#define FAT_CHAIN_END		0xffffffff
#define FAT_BLK_ENTRIES		(SECTOR_SIZE / sizeof(uint32_t))
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

struct rrfs_params {
    uint32_t tracks;
    uint32_t heads;
    uint32_t sectorspertrack;
    uint16_t bytespersector;
    uint32_t sectors;
    uint16_t bootsectors;
    uint32_t fatsectors;
    uint16_t sectorsperclust;
    uint32_t clusters;
} __attribute__ ((packed));

struct mbr {
    char jump[JUMP_SIZE];
    struct rrfs_params params;
    char boot[BOOTLDR_SIZE];
    char bootsig[BOOTSIG_SIZE];
} __attribute__ ((packed));

struct direntry {
    char name[DE_NAME_LENGTH];
    uint16_t attr;
    u_char time[DE_TIME_LENGTH];
    u_char date[DE_DATE_LENGTH];
    uint32_t size;
    uint32_t start;
} __attribute__ ((packed));

typedef struct mbr *mbr_t;
typedef struct direntry *direntry_t;

#endif
