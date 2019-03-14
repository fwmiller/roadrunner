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

void dump_fstab(char *s)
{
	fs_t fs;
	int fscnt = 0;
	int fsno;
	char s0[80];

	for (fsno = 0; fsno < FILE_SYSTEMS; fsno++)
		if (fstab[fsno].fsops != NULL) {
			if (fscnt++ == 0) {
				sprintf(s, "fsno  devno  blkno  path\n");
				s += strlen(s);
			}
			fs = &(fstab[fsno]);
			sprintf(s, "%4d    ", fsno);
			s += 8;

			if (fs->devno < 0)
				sprintf(s0, "  -      -  ");
			else
				sprintf(s0, "%3d  %5d  ", fs->devno,
					(u_int) fs->blkno);
			strcat(s, s0);
			s += strlen(s0);

			sprintf(s0, "%s\n", fs->path);
			strcat(s, s0);
			s += strlen(s0);
		}
}
