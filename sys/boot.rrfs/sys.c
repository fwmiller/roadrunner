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

int bcd2int(u_char v)
{
	return (int)((v >> 4) * 10 + (v % 0x10));
}

u_char loadbyte(u_char * ptr)
{
	return *ptr;
}

u_int loadword(u_char * ptr)
{
	return (u_int) * ptr | (u_int) (*(ptr + 1) << 8);
}

u_int loaddword(u_char * ptr)
{
	return (u_int) * ptr |
	    (u_int) (*(ptr + 1) << 8) |
	    (u_int) (*(ptr + 2) << 16) | (u_int) (*(ptr + 3) << 24);
}

void storebyte(u_char val, u_char * ptr)
{
	*ptr = (u_char) val & 0xff;
}

void storeword(u_int val, u_char * ptr)
{
	*ptr = (u_char) val & 0xff;
	*(ptr + 1) = (u_char) (val >> 8) & 0xff;
}

void storedword(u_int val, u_char * ptr)
{
	*ptr = (u_char) val & 0xff;
	*(ptr + 1) = (u_char) (val >> 8) & 0xff;
	*(ptr + 2) = (u_char) (val >> 16) & 0xff;
	*(ptr + 3) = (u_char) (val >> 24) & 0xff;
}
