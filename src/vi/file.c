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
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "vi.h"

int file_exists(char *path)
{
	int fd;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return 0;
	close(fd);
	return 1;
}

int file_open(char *path)
{
	char fbuf[SECTOR_SIZE];
	int fd, i, len;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return fd;

	for (;;) {
		len = read(fd, fbuf, SECTOR_SIZE);
		if (len <= 0)
			break;
		for (i = 0; i < len; i++)
			if (isprint(fbuf[i]) || fbuf[i] == ' '
			    || fbuf[i] == '\n') {
				incr_buflen();
				buf[bufpos++] = fbuf[i];
			}
	}
	close(fd);
	bufpos = 0;

	if (len < 0)
		return len;
	return 0;
}

int file_save(char *path)
{
	int fd, result;

	fd = open(path, O_WRONLY | O_CREAT);
	if (fd < 0)
		return fd;

	result = write(fd, buf, buflen);
	close(fd);
	return result;
}
