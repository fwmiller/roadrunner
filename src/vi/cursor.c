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

#include <ctype.h>
#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "vi.h"

static void prev_line()
{
	if (y == 0 && home > 0) {
		int dist;

		if (buf[home - 1] == '\n')
			home--;
		dist = dist_to_start_of_line(home);

		if (dist < width)
			home -= dist;
		else {
			int len;

			len = dist % width;
			if (len == 0)
				home -= width;
			else
				home -= len;
		}
		refresh();

	} else
		y--;
}

static void next_line()
{
	if (y == height - 1) {
		int len;

		len = dist_to_end_of_line(home);
		if (len < width)
			home += len;
		else
			home += width;

		if (buf[home] == '\n')
			home++;

		refresh();

	} else
		y++;
}

void cursor_on()
{
	char ch;

	move(y, x);
	ioctl(STDOUT, SET_REVERSE_VIDEO, NULL);
	ch = inch();
	addch(isprint(ch) ? ch : ' ');
	ioctl(STDOUT, SET_NORMAL_VIDEO, NULL);
}

void cursor_off()
{
	char ch;

	move(y, x);
	ioctl(STDOUT, SET_NORMAL_VIDEO, NULL);
	ch = inch();
	addch(isprint(ch) ? ch : ' ');
}

void cursor_home()
{
	x = 0;
	y = 0;
}

void cursor_up()
{
	int dist, len;

	dist = dist_to_start_of_line(bufpos);
	if (bufpos - (dist % width) == 0)
		return;

	cursor_off();

	if (dist >= width) {
		bufpos -= width;
		prev_line();

	} else {
		bufpos -= dist;
		if (bufpos > 0 && buf[bufpos - 1] == '\n')
			bufpos--;
		len = dist_to_start_of_line(bufpos) % width;

		if (len <= x) {
			x = len;
			if (len > 0) {
				x--;
				bufpos--;
			}
		} else
			bufpos -= len - x;

		prev_line();
	}
	status();
	cursor_on();
}

void cursor_down()
{
	int dist, len;

	dist = dist_to_end_of_line(bufpos);
	if (bufpos + (dist % width) == buflen)
		return;

	cursor_off();

	if (dist > width) {
		bufpos += width;
		next_line();

	} else {
		int rem;

		rem = width - x;
		if (dist >= rem) {
			if (dist - 1 > 0)
				bufpos += dist - 1;
		} else
			bufpos += dist + 1;

		next_line();

		if (dist >= rem)
			x = dist - rem - 1;

		else {
			len = dist_to_end_of_line(bufpos) % width;
			if (len <= x) {
				if (len - 1 > 0) {
					bufpos += len - 1;
					x = len - 1;
				} else
					x = 0;
			} else
				bufpos += x;
		}
	}

	status();
	cursor_on();
}

void cursor_left()
{
	if (bufpos == 0)
		return;

	cursor_off();

	bufpos--;

	if (x == 0) {
		prev_line();

		if (buf[bufpos] == '\n') {
			x = dist_to_start_of_line(bufpos) % width;
			if (x > 0) {
				x--;
				bufpos--;
			}
		} else
			x = width - 1;

	} else
		x--;

	status();
	cursor_on();
}

void cursor_right()
{
	if (bufpos >= buflen)
		return;

	cursor_off();

	if (buf[bufpos] == '\n') {
		next_line();
		x = 0;

	} else if (buf[bufpos + 1] == '\n') {
		next_line();
		x = 0;
		bufpos++;

	} else if (x == width - 1) {
		next_line();
		x = 0;

	} else
		x++;

	if (bufpos < buflen)
		bufpos++;

	status();
	cursor_on();
}
