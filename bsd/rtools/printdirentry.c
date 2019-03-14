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
#include <stdio.h>
#include "rrfs.h"

void printdirentry(direntry_t de)
{
	int hour;

	if (de->attr & DE_ATTR_DIR)
		printf("d");
	else
		printf("-");
	if (de->attr & DE_ATTR_READ)
		printf("r");
	else
		printf("-");
	if (de->attr & DE_ATTR_WRITE)
		printf("w");
	else
		printf("-");
	if (de->attr & DE_ATTR_EXEC)
		printf("x");
	else
		printf("-");

	printf(" %8u", (uint32_t) de->size);

	hour = (int)de->time[DE_TIME_HOUR];
	if (hour == 0)
		hour = 24;
	printf(" %2d", (hour > 12 ? hour - 12 : hour));

	printf(":%02d:%02d %2d-%02d-%04d",
	       (int)de->time[DE_TIME_MIN],
	       (int)de->time[DE_TIME_SEC],
	       (int)de->date[DE_DATE_MON] + 1,
	       (int)de->date[DE_DATE_DAY], (int)de->date[DE_DATE_YEAR] + 1900);

	printf(" %6u %s\n", (uint32_t) de->start, de->name);
}
