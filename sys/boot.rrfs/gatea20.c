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

/* The gateA20() function was taken from the FreeBSD 2.2.1 distribution */
#define K_RDWR		0x60	       /* Keyboard data & cmds (rd/wr) */
#define K_STATUS	0x64	       /* Keyboard status */
#define K_CMD		0x64	       /* Keybd ctlr command (wr-only) */

#define K_OBUF_FUL	0x01	       /* Output buffer full */
#define K_IBUF_FUL	0x02	       /* Input buffer full */

#define KC_CMD_WOUT	0xd1	       /* Write output port */
#define KB_A20		0xdf	       /* Enable A20 line */

void
gateA20()
{
    while (inb(K_STATUS) & K_IBUF_FUL);
    while (inb(K_STATUS) & K_OBUF_FUL)
	inb(K_RDWR);

    outb(K_CMD, KC_CMD_WOUT);
    while (inb(K_STATUS) & K_IBUF_FUL);
    outb(K_RDWR, KB_A20);
    while (inb(K_STATUS) & K_IBUF_FUL);
}
