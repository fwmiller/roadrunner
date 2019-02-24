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
#include <fs/rrfs.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mem.h>

int
rrfile_attr(file_t file, attrlist_t l)
{
    int i, size;

    l->n = RRFS_FILE_ATTRIBUTES;
    l->key = RRFS_FILE_KEY_ATTR;

    size = l->n * (sizeof(attr_t) + sizeof(struct attr));

    if ((l->attr = (attr_t *) malloc(size)) == NULL)
	return ENOMEM;
    bzero(l->attr, size);

    for (i = 0; i < RRFS_FILE_ATTRIBUTES; i++) {
	l->attr[i] = (attr_t) ((u_long) l->attr +
			       (l->n * sizeof(attr_t)) +
			       (i * sizeof(struct attr)));

	switch (i) {
	case 0:
	    l->attr[i]->type = ATTR_STRING;
	    l->attr[i]->len = 4;
	    strcpy(l->attr[i]->name, "attributes");
	    break;
	case 1:
	    l->attr[i]->type = ATTR_UINT;
	    l->attr[i]->len = sizeof(u_int);
	    strcpy(l->attr[i]->name, "size");
	    break;
	case 2:
	    l->attr[i]->type = ATTR_STRING;
	    l->attr[i]->len = 22;
	    strcpy(l->attr[i]->name, "time");
	    break;
	case 3:
	    l->attr[i]->type = ATTR_STRING;
	    l->attr[i]->len = 48;
	    strcpy(l->attr[i]->name, "name");
	    break;

	//default:
	}
    }
#if _DEBUG
    for (size = 0, i = 0; i < RRFS_FILE_ATTRIBUTES; i++)
	size += l->attr[i]->len;
    kprintf("rrfile_attr: n %d size %d\n", l->n, size);
#endif

    return 0;
}
