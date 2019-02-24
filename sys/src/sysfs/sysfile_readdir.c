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
#include <fs/sysfs.h>
#include <string.h>
#include <sys/intr.h>

/*
 * System file system group member attributes include the following:
 *
 * string[SYSFILE_NAME_LEN]	Name
 */
struct gmentry {
    char name[SYSFILE_NAME_LEN];
} __attribute__ ((packed));

typedef struct gmentry *gmentry_t;

int
sysfile_readdir(file_t file, char *entry)
{
    gmentry_t gm;
    int i;

    disable;

    i = (int) file->data;
    if (i == SYSFILE_NAMES) {
	enable;
	return EFILEEOF;
    }
    gm = (gmentry_t) entry;
    strcpy(gm->name, sysfs_filenames[i++]);
    file->data = (void *) i;

    enable;
    return 0;
}
