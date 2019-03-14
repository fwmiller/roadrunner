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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int sysfile_attr(file_t file, attrlist_t l)
{
	int size;

#if _DEBUG
	int i;
#endif

	l->n = SYSFS_FILE_ATTRIBUTES;
	l->key = SYSFS_FILE_KEY_ATTR;

	size = l->n * (sizeof(attr_t) + sizeof(struct attr));

	if ((l->attr = (attr_t *) malloc(size)) == NULL)
		return ENOMEM;
	bzero(l->attr, size);

	l->attr[0] = (attr_t) ((u_long) l->attr + sizeof(struct attr));

	l->attr[0]->type = ATTR_STRING;
	l->attr[0]->len = SYSFILE_NAME_LEN;
	strcpy(l->attr[0]->name, "name");

#if _DEBUG
	for (size = 0, i = 0; i < SYSFS_FILE_ATTRIBUTES; i++)
		size += l->attr[i]->len;
	kprintf("sysfile_attr: n %d size %d\n", l->n, size);
#endif

	return 0;
}
