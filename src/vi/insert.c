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
#include <string.h>
#include <sys/config.h>
#include "vi.h"

/* Insert a newline in the buffer */
static int
insert_newline()
{
    int result;

    if ((result = incr_buflen()) < 0)
	return result;

    cursor_off();

    if (bufpos < buflen - 1)
	shift_right(buf + bufpos + 1, buf + bufpos, buflen - bufpos - 1);
    buf[bufpos++] = '\n';
    bufdirty = 1;

    if (y == width - 1) {
	home += dist_to_end_of_line(home);
	if (buf[home] == '\n')
	    home++;
	refresh();
	x = 0;
	clrtoeol();

    } else {
	clrtoeol();
	y++;
	x = 0;
	refresh();
    }

    cursor_on();
    return 0;
}

/* Insert a character other than a newline in the buffer */
static int
insert_char(char ch)
{
    int result;

    if ((result = incr_buflen()) < 0)
	return result;

    cursor_off();

    /* Make room for the inserted character */
    if (bufpos < buflen - 1)
	shift_right(buf + bufpos + 1, buf + bufpos, buflen - bufpos - 1);

    /* Insert character into buffer and mark buffer dirty */
    buf[bufpos++] = ch;
    bufdirty = 1;

    /* Display inserted character at current cursor position */
    addch(ch);

    /* Advance cursor position */
    if (x == width - 1) {
	/* At end of current line */
	if (y == height - 1) {
	    int dist;

	    /* Scroll up for end of screen */
	    dist = dist_to_end_of_line(home);
	    if (dist >= width)
		home += width;
	    else
		home += dist;

	    /* Advance past a newline if necessary */
	    if (buf[home] == '\n')
		home++;
	} else
	    y++;

	/* Cursor moves to beginning of next line */
	x = 0;

    } else
	x++;

    refresh();
    cursor_on();
    return 0;
}

void
insert()
{
    char ch;

    while ((ch = getch()) != ESC)
	if (ch == '\n')
	    insert_newline();
	else
	    insert_char(ch);
}
