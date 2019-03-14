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

#include <limits.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/tsc.h>

static struct tsc cpuspeed;

void tscsub(tsc_t result, tsc_t cnt1, tsc_t cnt2)
{
	result->hi = 0;
	result->lo = 0;

	if (cnt1->hi == cnt2->hi && cnt1->lo > cnt2->lo)
		result->lo += cnt1->lo - cnt2->lo;

	else if (cnt1->hi > cnt2->hi) {
		result->lo += (UINT_MAX - cnt2->lo);

		if (cnt1->lo <= UINT_MAX - result->lo)
			result->lo += cnt1->lo + 1;
		else {
			result->lo = cnt1->lo - (UINT_MAX - result->lo);
			result->hi++;
		}

		result->hi += cnt1->hi - cnt2->hi - 1;
	}
}

void tsccalibrate()
{
	struct timeval tv1, tv2;
	struct tsc cnt1, cnt2;

	utime(&(tv1.tv_sec), &(tv1.tv_usec));
	for (;;) {
		utime(&(tv2.tv_sec), &(tv2.tv_usec));
		if (tv2.tv_sec != tv1.tv_sec)
			break;
	}
	tscread(&cnt1);
	for (;;) {
		utime(&(tv1.tv_sec), &(tv1.tv_usec));
		if (tv1.tv_sec != tv2.tv_sec)
			break;
	}
	tscread(&cnt2);
	tscsub(&cpuspeed, &cnt2, &cnt1);

	if (cpuspeed.hi > 0)
		kprintf("cpu is faster than 4 GHz\n");
	else if (cpuspeed.lo > 1000000000)
		kprintf("cpu speed %d.%d GHz\n",
			cpuspeed.lo / 1000000000,
			(cpuspeed.lo % 1000000000) / 1000000);
	else
		kprintf("cpu speed %d MHz\n", cpuspeed.lo / 1000000);
}
