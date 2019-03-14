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
#include <string.h>
#include <sys/config.h>
#include <sys/lex.h>
#include "vi.h"

/* LINE_LENGTH must be greater than width */
static char cmdline[LINE_LENGTH];

static void get_command()
{
	int tempx = x, tempy = y;
	int pos;
	char ch;

	move(height, 0);
	clrtoeol();
	addch(':');

	bzero(cmdline, LINE_LENGTH);
	for (pos = 0;;) {
		ch = getchar();
		if (ch == '\n')
			break;
		if (ch == '\b')
			continue;
		if (pos >= width - 2)
			continue;
		cmdline[pos++] = ch;
		move(height, pos);
		addch(ch);
	}
	y = tempy;
	x = tempx;
	move(y, x);
}

static void do_command()
{
	char arg[LINE_LENGTH];
	int pos = 0;

	bzero(arg, LINE_LENGTH);
	nextarg(cmdline, &pos, arg);

	if (arg[0] == 'w') {
		char s[LINE_LENGTH];
		int result;

		bzero(arg, LINE_LENGTH);
		nextarg(cmdline, &pos, arg);

		if (file_exists(arg)) {
			bzero(s, LINE_LENGTH);
			sprintf(s, "file exists");
			message(s);
			return;
		}
		bzero(s, LINE_LENGTH);
		sprintf(s, "write %s", arg);
		message(s);

		result = file_save(arg);
		if (result < 0) {
			bzero(s, LINE_LENGTH);
			sprintf(s, "write failed (%s)", strerror(result));
			message(s);
		}
		return;
	}
	message("illegal command");
}

void command()
{
	get_command();
	do_command();
}
