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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "vi.h"

#define BUFLEN_INCR	PAGE_SIZE

#define VM_TRANSLATE	0
#define VM_INSERT	1
#define VM_COMMAND	2

/* Buffer attributes */
char *buf = NULL;
int buflen = 0, maxbuflen = PAGE_SIZE, bufpos = 0, home = 0, bufdirty = 0;

/* Screen dimensions */
int width = 0, height = 0;

/* Cursor position */
int x = 0, y = 0;

/* Mode */
static int mode = VM_TRANSLATE;

int
incr_buflen()
{
    if (buflen + 1 == maxbuflen) {
	char *tmp;

	tmp = (char *) malloc(maxbuflen + BUFLEN_INCR);
	if (tmp == NULL)
	    return ENOMEM;
	bzero(tmp, maxbuflen + BUFLEN_INCR);
	maxbuflen += BUFLEN_INCR;
	bcopy(buf, tmp, buflen);
	free(buf);
	buf = tmp;
    }
    buflen++;
    return 0;
}

void
status()
{
    int tempx = x, tempy = y;
    char s[80];

    move(height, 0);
    clrtoeol();
    ioctl(STDOUT, SET_BRIGHT_VIDEO, NULL);

    bzero(s, 80);
    sprintf(s, "home %d bufpos %d x %d y %d", home, bufpos, x, y);
    addstr(s);

    ioctl(STDOUT, SET_NORMAL_VIDEO, NULL);
    y = tempy;
    x = tempx;
    move(y, x);
}

void
message(char *s)
{
    int tempx = x, tempy = y;

    move(height, 0);
    clrtoeol();
    ioctl(STDOUT, SET_BRIGHT_VIDEO, NULL);
    addstr(s);
    ioctl(STDOUT, SET_NORMAL_VIDEO, NULL);
    y = tempy;
    x = tempx;
    move(y, x);
}

static void
vi()
{
    int getnextchar = 1;
    char ch;

    for (;;) {
	if (getnextchar)
	    ch = getchar();
	else
	    getnextchar = 1;

	switch (mode) {
	case VM_TRANSLATE:
	    if (ch == 'Z') {
		ch = getchar();
		if (ch == 'Z')
		    return;
		else
		    getnextchar = 0;
	    } else if (ch == 'h')
		cursor_left();
	    else if (ch == 'i') {
		mode = VM_INSERT;
		getnextchar = 0;
	    } else if (ch == 'j')
		cursor_down();
	    else if (ch == 'k')
		cursor_up();
	    else if (ch == 'l')
		cursor_right();
	    else if (ch == 'x')
		delete();
	    else if (ch == ':') {
		mode = VM_COMMAND;
		getnextchar = 0;
	    } else if (ch == '?')
		help();
	    break;

	case VM_INSERT:
	    insert();
	    mode = VM_TRANSLATE;
	    break;

	case VM_COMMAND:
	    command();
	    mode = VM_TRANSLATE;
	    break;

	default:;
	}
    }
}

int
main(int argc, char **argv)
{
    int result;

    buf = (char *) malloc(maxbuflen);
    if (buf == NULL) {
	printf("could not allocate text buffer\n");
	return ENOMEM;
    }

    if (argc > 2) {
	printf("too many arguments\n");
	free(buf);
	return EINVAL;

    } else if (argc == 2) {
	result = file_open(argv[1]);
	if (result < 0) {
	    printf("could not open %s (%s)\n", argv[1], strerror(result));
	    free(buf);
	    return result;
	}
    }
    ioctl(STDOUT, GET_WINDOW_WIDTH, &width);
    ioctl(STDOUT, GET_WINDOW_HEIGHT, &height);

    /* Leave the last line for status and messages */
    height--;

    /* Turn off operating system cursor */
    ioctl(STDOUT, SET_CURSOR_OFF, NULL);

    /* Setup initial display */
    clear();
    refresh();
    cursor_home();

    /* Turn on vi cursor */
    cursor_on();

    /* Main processing loop */
    vi();

    /* Turn off vi cursor */
    cursor_off();

    /* Clear screen before exiting */
    clear();

    /* Turn on operating system cursor */
    ioctl(STDOUT, SET_CURSOR_ON, NULL);

    free(buf);
    return 0;
}
