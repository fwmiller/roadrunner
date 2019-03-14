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
#include <fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/config.h>
#include <unistd.h>

static void copy(const char *src, const char *dst)
{
	char buf[SECTOR_SIZE];
	int fd1, fd2, len;

	fd2 = open(dst, O_RDONLY);
	if (fd2 >= 0) {
		printf("%s exists\n", dst);
		close(fd2);
		return;
	}
	fd1 = open(src, O_RDONLY);
	if (fd1 < 0) {
		printf("could not open %s (%s)\n", src, strerror(fd1));
		return;
	}
	fd2 = open(dst, O_WRONLY | O_CREAT);
	if (fd2 < 0) {
		printf("could not open %s (%s)\n", dst, strerror(fd2));
		close(fd1);
		return;
	}

	while ((len = read(fd1, buf, SECTOR_SIZE)) > 0) {
		len = write(fd2, buf, (size_t) len);
		if (len < 0) {
			printf("write failed (%s)\n", strerror(len));
			close(fd2);
			close(fd1);
			return;
		}
	}
	if (len < 0)
		printf("read failed (%s)\n", strerror(len));
	close(fd2);
	close(fd1);
}

int main(int argc, char **argv)
{
	char path[LINE_LENGTH], name[LINE_LENGTH];
	int isdir = 0, i, result;

	/* Command line arguments */
	if (argc < 3 || argv == NULL) {
		printf("missing arguments\n");
		return EINVAL;
	}
	getcwd(path, LINE_LENGTH);

	/* Check whether last argument is a directory */
	result = chdir(argv[argc - 1]);
	if (result == 0) {
		chdir(path);
		isdir = 1;
	}
	/* Special case for copying a single file */
	if (!isdir) {
#if _DEBUG
		printf("cp: %s is not a directory\n", argv[argc - 1]);
#endif
		if (argc > 3) {
			printf("too many arguments\n");
			return EINVAL;
		}
		copy(argv[1], argv[2]);
	} else {
#if _DEBUG
		printf("cp: %s is a directory\n", argv[argc - 1]);
#endif
		/* Copy each file to the destination directory */
		for (i = 1; i < argc - 1; i++) {
			bzero(name, LINE_LENGTH);
			getname(argv[i], name);
			mkpath(argv[argc - 1], name, path);
#if _DEBUG
			printf("cp: %s --> %s\n", argv[i], path);
#endif
			copy(argv[i], path);
		}
	}
	return 0;
}
