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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
has_pattern(char *path)
{
    int i, len;

    for (len = strlen(path), i = 0; i < len; i++)
	if (path[i] == '*')
	    return i;
    return (-1);
}

/*
 * The pattern is a path with the pattern characters in one element of the
 * path.  Break the path into three parts, a prefix to the element containing
 * the pattern, the element containing the pattern, and a suffix to the
 * element containing the pattern.
 */
int
pattern_break(char *path, int pos, char **prefix,
	      char **pattern, char **suffix)
{
    int i, j;

    *prefix = NULL;
    *pattern = NULL;
    *suffix = NULL;

    *prefix = (char *) malloc(PATH_LENGTH);
    if (*prefix == NULL)
	return ENOMEM;

    *pattern = (char *) malloc(PATH_LENGTH);
    if (*pattern == NULL) {
	free(*prefix);
	*prefix = NULL;
	return ENOMEM;
    }
    *suffix = (char *) malloc(PATH_LENGTH);
    if (*suffix == NULL) {
	free(*pattern);
	*pattern = NULL;
	free(*prefix);
	*prefix = NULL;
	return ENOMEM;
    }
    bzero(*prefix, PATH_LENGTH);
    bzero(*pattern, PATH_LENGTH);
    bzero(*suffix, PATH_LENGTH);

    for (i = pos;;)
	if (--i < 0 || path[i] == '/')
	    break;
    if (i >= 0) {
	strncpy(*prefix, path, i);
    }
    for (j = pos + 1;; j++)
	if (j >= PATH_LENGTH || path[j] == '\0' || path[j] == '/')
	    break;

    if (i < 0)
	i = 0;
    else
	i = i + 1;

    strncpy(*pattern, path + i, j - i);
    strncpy(*suffix, path + j, strlen(path) - j);
    return 0;
}
