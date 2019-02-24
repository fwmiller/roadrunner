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

#ifndef __VI_H
#define __VI_H

/* Screen file descriptor */
extern int stdscr;

/* Buffer attributes */
extern char *buf;
extern int buflen, maxbuflen, bufpos, home, bufdirty;

/* Screen dimensions */
extern int width, height;

/* Cursor position */
extern int x, y;

void command();
void cursor_down();
void cursor_home();
void cursor_left();
void cursor_off();
void cursor_on();
void cursor_right();
void cursor_up();
int dist_to_start_of_line(int pos);
int dist_to_end_of_line(int pos);
int file_exists(char *path);
int file_open(char *path);
int file_save(char *path);
void help();
int incr_buflen();
void insert();
void delete();
void message(char *s);
void refresh();
void shift_left(char *dst, char *src, int len);
void shift_right(char *dst, char *src, int len);
void status();

#endif
