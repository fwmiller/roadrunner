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

#include <curses.h>
#include "vi.h"

/* Redraw the screen starting at home */
void
refresh()
{
    int x, y, i;

    /*
     * Draw characters from buffer until the buffer is exhausted or the
     * end of the screen is reached
     */
    for (x = 0, y = 0, i = home; i < buflen; i++) {
	move(y, x);

	if (buf[i] == '\n')
	    clrtoeol();
	else
	    addch(buf[i]);

	if (buf[i] == '\n' || x == width - 1) {
	    /* Check for end of screen */
	    if (y == height - 1)
		break;

	    /* Begining of next line */
	    y++;
	    x = 0;
	    clrtoeol();

	} else
	    /* Next position on the same line */
	    x++;
    }

    /* Clean up last line on which characters were displayed */
    move(y, x);
    clrtoeol();

    /*
     * Fillers at the beginning of each unused line if the buffer does not
     * fill the screen
     */
    for (y++; y < height; y++) {
	move(y, 0);
	addch('~');
	clrtoeol();
    }
}
