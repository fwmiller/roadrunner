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

#include <sys/types.h>

void put(int c);

void printf(const char *format, ...)
{
	int *dataptr = (int *)&format;
	char c;

	dataptr++;
	while ((c = *format++))
		if (c != '%')
			put(c);
		else
			switch (c = *format++) {
			case 'd':
				{
					int num = *dataptr++;
					char buf[10], *ptr = buf;

					if (num < 0) {
						num = -num;
						put('-');
					}
					do
						*ptr++ = '0' + num % 10;
					while (num /= 10);
					do
						put(*--ptr);
					while (ptr != buf);
					break;
				}

			case 'u':
				{
					u_int num = *dataptr++;
					char buf[10], *ptr = buf;

					if (num < 0) {
						num = -num;
						put('-');
					}
					do
						*ptr++ = '0' + num % 10;
					while (num /= 10);
					do
						put(*--ptr);
					while (ptr != buf);
					break;
				}

			case 'x':
				{
					unsigned int num = *dataptr++, dig;
					char buf[8], *ptr = buf;

					do
						*ptr++ = (dig =
							  (num & 0xf)) >
						    9 ? 'a' + dig - 10 : '0' +
						    dig;
					while (num >>= 4);
					do
						put(*--ptr);
					while (ptr != buf);
					break;
				}

			case 's':
				{
					char *ptr = (char *)*dataptr++;

					while ((c = *ptr++))
						put(c);
					break;
				}
			case 'c':
			default:
				put((*dataptr++) & 0xff);
			}
}
