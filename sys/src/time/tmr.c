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

#include <sys.h>
#include <sys/config.h>
#include <sys/i8254.h>
#include <sys/time.h>
#include <sys/tmr.h>

#define DIV16(val) ((val >> 16) > 0 ? 0xffff : val)

u_int tmrcount(u_int freq)
{
	if (freq == 0)
		return (u_int) (I8254_CLK / CLOCK);
	return (u_int) DIV16(I8254_CLK / freq);
}

u_int tmrtick()
{
	return tick;
}

void tmrstart(u_int count)
{
	outb(I8254_CTRL, I8254_CNTR_0_START);
	outb(I8254_CNTR_0, count);
	outb(I8254_CNTR_0, count >> 8);
}

u_int tmrread()
{
	u_int count = 0;

	outb(I8254_CTRL, I8254_CNTR_0_LATCH);
	count = inb(I8254_CNTR_0);
	count <<= 8;
	count = inb(I8254_CNTR_0);
	return count;
}
