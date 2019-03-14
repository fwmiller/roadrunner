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

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int fd, result;

	if (argc != 2)
		return EINVAL;

	fd = open(argv[1], O_RDONLY);
	if (fd >= 0) {
		printf("%s exists\n", argv[1]);
		close(fd);
		return EINVAL;
	}

	result = open(argv[1], O_WRONLY | O_CREAT | O_MKDIR);
	if (result < 0) {
		printf("could not create directory %s (%s)\n",
		       argv[1], strerror(result));
		return result;
	}
	fd = result;
	close(fd);
	return 0;
}
