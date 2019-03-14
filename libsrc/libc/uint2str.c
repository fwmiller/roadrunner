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

void uint2str(u_int v, char *s, int base)
{
	if (v == 0) {
		s[0] = '0';
		s[1] = '\0';
	} else {
		char s1[80];
		u_int v1;
		int i = 0, j = 0;

		while (v > 0) {
			v1 = v % base;
			if (v1 < 10)
				s1[i++] = v1 + '0';
			else
				s1[i++] = v1 - 10 + 'a';
			v /= base;
		}
		while (i > 0)
			s[j++] = s1[--i];
		s[j] = '\0';
	}
}
