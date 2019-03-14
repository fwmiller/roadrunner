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

#include <fs.h>
#if _DEBUG
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <sys/mem.h>

void getdir(char *path, char *dir)
{
	int len, i;

	if (strlen(path) == 0) {
#if _DEBUG
		kprintf("getdir: zero length path\n");
#endif
		strcpy(dir, "");
		return;
	}
	if (strcmp(path, "/") == 0) {
#if _DEBUG
		kprintf("getdir: root directory\n");
#endif
		strcpy(dir, "/");
		return;
	}
	i = strlen(path);
	while (path[i - 1] == '/')
		i--;

	while (i > 0 && path[--i] != '/') ;
	strncpy(dir, path, i + 1);
	dir[i + 1] = '\0';

	if (strcmp(dir, "/") != 0 && dir[(len = strlen(dir)) - 1] == '/')
		dir[len - 1] = '\0';
}

void getname(char *path, char *name)
{
	int i, len;

	if (strlen(path) == 0 || strcmp(path, "/") == 0) {
		strcpy(name, "");
		return;
	}
	len = strlen(path);
	while (path[len - 1] == '/')
		len--;
	i = len - 1;

	for (i = len - 1; i >= 0; i--)
		if (path[i] == '/')
			break;
	strncpy(name, path + i + 1, len - i - 1);
	name[len - i - 1] = '\0';
}

#define ELEM_LENGTH	132

void mkpath(char *dir, char *name, char *path)
{
	char elem[ELEM_LENGTH];
	int pos, start, len;

	if (dir == NULL || name == NULL || path == NULL)
		return;

	for (strcpy(path, dir), pos = 0; name[pos] != '\0';) {
		while (name[pos] == '/')
			pos++;

		for (start = pos; name[pos] != '/' && name[pos] != '\0'; pos++)
			len = pos - start + 1;
		strncpy(elem, name + start,
			(len < ELEM_LENGTH - 1 ? len : ELEM_LENGTH - 1));
		elem[(len < ELEM_LENGTH - 1 ? len : ELEM_LENGTH - 1)] = '\0';

		if (strcmp(elem, "..") == 0) {
			char *s;

			if ((s = (char *)malloc(PATH_LENGTH)) == NULL)
				return;

			strcpy(s, path);
			getdir(s, path);
			free(s);

		} else if (strcmp(elem, ".") != 0) {
			len = strlen(path);
			if (len == 0)
				strcpy(path, elem);
			else if (path[len - 1] == '/') {
				if (elem[0] == '/')
					strcat(path, elem + 1);
				else
					strcat(path, elem);
			} else {
				if (elem[0] == '/')
					strcat(path, elem);
				else {
					strcat(path, "/");
					strcat(path, elem);
				}
			}
		}
	}
	if (strcmp(path, "/") != 0 && path[(len = strlen(path)) - 1] == '/')
		path[len - 1] = '\0';
}
