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

#include <dev.h>
#include <errno.h>
#include <fs/devfs.h>
#include <stdio.h>
#include <string.h>
#include <sys/config.h>
#include <sys/intr.h>

/* 
 * Device file system group member attributes include the following:
 *
 * string[8]    Type
 * int          Reference count
 * string[32]   Name
 */
struct gmentry {
    char type[8];
    int refcnt;
    char name[32];
} __attribute__ ((packed));

typedef struct gmentry *gmentry_t;

int
devfile_readdir(file_t file, char *entry)
{
    gmentry_t gm;
    dev_t dev;
    int i;

    disable;

    gm = (gmentry_t) entry;

    for (i = (int) file->data; i < DEVICES; i++) {
	dev = &(devtab[i]);

	if (strcmp(dev->name, "") != 0) {
	    if (dev->type == DEV_TYPE_CHAR)
		sprintf(gm->type, "char    ");
	    else if (dev->type == DEV_TYPE_BLK)
		sprintf(gm->type, "blk     ");
	    else
		sprintf(gm->type, "ERROR   ");

	    gm->refcnt = dev->refcnt;
	    strncpy(gm->name, dev->name, DEV_NAME_LEN);

	    file->data = (void *) (++i);
	    enable;
	    return 0;
	}
    }
    enable;
    return EFILEEOF;
}
