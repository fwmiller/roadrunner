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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_HDR "fileno  fsno  filesize       pos  bufsize  refcnt  path"

void dump_filetab(char *s)
{
	file_t file;
	int filecnt = 0;
	int fileno;
	char fullpath[132];
	char s0[80];

	for (fileno = 0; fileno < FILES; fileno++)
		if (filetab[fileno].fs != NULL) {
			if (filecnt++ == 0) {
				sprintf(s, "%s\n", FILE_HDR);
				s += strlen(s);
			}
			file = &(filetab[fileno]);

			if (file->type == FT_SOCKET)
				strcpy(fullpath, "socket");
			else
				mkpath(file->fs->path, file->path, fullpath);

			sprintf(s, "  %4d  ", fileno);
			s += 8;

			if (file->type == FT_SOCKET)
				sprintf(s, "   -  ");
			else
				sprintf(s, "%4d  ", file->fs->slot);
			s += 6;

			if (file->filesize == MAX_FILE_SIZE
			    || file->type == FT_SOCKET)
				sprintf(s, "       -");
			else
				sprintf(s, "%8u", (u_int) file->filesize);
			s += 8;

			if (file->type == FT_SOCKET)
				sprintf(s, "         -        -  ");
			else
				sprintf(s, "  %8u    %5u  ",
					(u_int) file->pos, file->bufsize);
			s += 21;

			sprintf(s0, "%6d  %s\n", file->refcnt, fullpath);
			strcat(s, s0);
			s += strlen(s0);
		}
}
