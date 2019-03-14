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

#include <dev.h>
#include <errno.h>
#include <fs/devfs.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/lex.h>

static int parse(char *path, lex_t devname)
{
	struct lex l;
	int pos = 0;

	nextlex(path, &pos, &l);
	if (l.type != LEX_SLASH)
		return EINVAL;
	nextlex(path, &pos, devname);
	if (devname->type != LEX_ID)
		return EINVAL;
	nextlex(path, &pos, &l);
	if (l.type != LEX_EOL)
		return EINVAL;
	return 0;
}

int devfile_open(file_t file)
{
	struct lex devname;
	int devno, result;

	/* Special case for root directory */
	if (strcmp(file->path, "/") == 0 || strcmp(file->path, "") == 0) {
		file->data = (void *)0;
		file->flags |= F_EOF | F_DIR;
		return 0;
	}
	result = parse(file->path, &devname);
	if (result < 0) {
		file->flags |= F_ERR;
		return result;
	}
	devno = dev_open(devname.s);
	if (devno < 0) {
		file->flags |= F_ERR;
		return devno;
	}
	file->filesize = MAX_FILE_SIZE;
	file->data = (void *)devno;
	dev_ioctl(devno, GET_BUFFER_SIZE, &(file->bufsize));
	return 0;
}
