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

#include <curses.h>
#include <stdio.h>
#include "vi.h"

#define HELP_LINES	13

static char *help_text[HELP_LINES] = {
	"Command  Description",
	"ZZ       Exit",
	"h        Move cursor left",
	"i        Enter insert mode (ESC to exit insert mode)",
	"j        Move cursor down",
	"k        Move cursor up",
	"l        Move cursor right",
	"x        Delete character",
	":        Enter command mode",
	"           Command       Description",
	"           w <filename>  Save file as <filename>",
	"",
	"?        Help screen",
};

void help()
{
	int i;

	clear();
	for (i = 0; i < HELP_LINES; i++) {
		move(i, 0);
		addstr(help_text[i]);
	}
	getchar();
	clear();
	refresh();
}
