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
#include <sys/config.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	char buf[SECTOR_SIZE];
	size_t len;
	int fd, i, j, result;

#if _DEBUG
	printf("cat: argc %d argv 0x%08x\n", argc, argv);
#endif
	/* Command line arguments */
	for (i = 1; i < argc; i++) {
#if _DEBUG
		printf("cat: argv[%d] [%s]\n", i, argv[i]);
#endif
		/* XXX Need to check whether argv[i] is a directory */

		result = open(argv[i], O_RDONLY);
		if (result < 0) {
#if _DEBUG
			printf("cat: could not open %s (%s)\n",
			       argv[i], strerror(result));
#endif
			continue;
		}
		for (fd = result;;) {
			len = read(fd, buf, SECTOR_SIZE);
			if (len <= 0)
				break;

			for (j = 0; j < len; j++)
				if (isprint(buf[j]) || isspace(buf[j])
				    || buf[j] == '\n')
					printf("%c", buf[j]);
		}
		close(fd);
	}
	return 0;
}
