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

#include <inttypes.h>
#include <string.h>
#include <time.h>
#include "rrfs.h"

void printdirentry(direntry_t de);

void
rrfs_mkdirentry(direntry_t de,
		char *name, uint16_t attr, uint32_t size, uint32_t start)
{
	time_t t;
	struct tm *tm;

	bzero((char *)de, DE_SIZE);
	strncpy(de->name, name, DE_NAME_LENGTH);
	de->attr = attr;
	time(&t);
	tm = localtime(&t);
	de->time[DE_TIME_HOUR] = (u_char) tm->tm_hour;
	de->time[DE_TIME_MIN] = (u_char) tm->tm_min;
	de->time[DE_TIME_SEC] = (u_char) tm->tm_sec;
	de->date[DE_DATE_MON] = (u_char) tm->tm_mon;
	de->date[DE_DATE_DAY] = (u_char) tm->tm_mday;
	de->date[DE_DATE_YEAR] = (u_char) tm->tm_year;
	de->size = size;
	de->start = start;
#if _DEBUG
	printdirentry(de);
#endif
}
