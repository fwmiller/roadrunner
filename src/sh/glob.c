/*
 *  Roadrunner/pk
 *    Copyright (C) 1989-2002  Cornfed Systems, Inc.
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
#include <fcntl.h>
#include <fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int
match(char *name, char *pat, char *patend)
{
    char ch;

    while (pat < patend)
	switch (ch = *pat++) {
	case '*':
	    if (pat == patend)
		return 1;
	    do
		if (match(name, pat, patend))
		    return 1;
	    while (*name++ != '\0');
	    return 0;

	default:
	    if (ch != *name++)
		return 0;
	}
    return (*name == '\0' ? 1 : 0);
}

static void
glob_extend(char *name, char **pathv)
{
    char *newpathv;
    int newlen;

    if (*pathv == NULL) {
	*pathv = (char *) malloc(strlen(name) + 1);
	if (*pathv == NULL)
	    return;
	strcpy(*pathv, name);
	return;
    }
    newlen = strlen(name) + strlen(*pathv) + 1;
    if ((newpathv = (char *) malloc(newlen)) == NULL)
	return;

    strcpy(newpathv, *pathv);
    strcat(newpathv, " ");
    strcat(newpathv, name);

    free(*pathv);
    *pathv = newpathv;
}

int
glob(char *pattern, char *path, char **pathv)
{
    int dir, size, off, i, result;
    struct attrlist l;
    char *entry;

    *pathv = NULL;

#if 0
    if (chdir(path) < 0)
	return EFAIL;
#endif
    if ((dir = open(path, O_RDONLY)) < 0)
	return dir;
    if ((result = attr(dir, &l)) < 0) {
	close(dir);
	return result;
    }
    /*
     * For the moment, lets glob on the key attribute only.  It must be of
     * type ATTR_STRING and it's name must be name.
     */
    if (l.attr[l.key]->type != ATTR_STRING) {
	free(l.attr);
	close(dir);
	return EFAIL;
    }
    if (strcmp(l.attr[l.key]->name, "name") != 0) {
	free(l.attr);
	close(dir);
	return EFAIL;
    }
    /* Determine the size of each directory entry */
    for (size = 0, i = 0; i < l.n; i++)
	size += l.attr[i]->len;

    /* Determine the offset of key attribute */
    for (off = 0, i = 0; i < l.key; i++)
	off += l.attr[i]->len;

    entry = (char *) malloc(size);
    if (entry == NULL) {
	free(l.attr);
	close(dir);
	return ENOMEM;
    }
#define NAME ((char *) (entry + off))

    /* Search for matching names. */
    for (result = 0;;) {
	bzero(entry, size);
	if ((result = readdir(dir, entry)) < 0)
	    break;
	else if (match(NAME, pattern, pattern + strlen(pattern)))
	    glob_extend(NAME, pathv);
    }
    free(entry);
    free(l.attr);
    close(dir);
    if (result < 0 && result != EFILEEOF)
	return result;
    return 0;
}
