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

#include <string.h>

void getdir(char *path, char *dir)
{
	int len, i;

	if (strlen(path) == 0 || path[0] != '/') {
		strcpy(dir, "");
		return;
	}
	if (strcmp(path, "/") == 0) {
		strcpy(dir, "/");
		return;
	}
	i = strlen(path);
	if (path[i - 1] == '/')
		i--;

	for (;;)
		if (path[--i] == '/')
			break;
	strncpy(dir, path, i + 1);
	dir[i + 1] = '\0';

	if (strcmp(dir, "/") != 0 && dir[(len = strlen(dir)) - 1] == '/')
		dir[len - 1] = '\0';
}
