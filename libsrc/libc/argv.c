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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/lex.h>

int argv_alloc(char *cmd, int *argc, char ***argv)
{
	char *arg, *ptr;
	int argvlen, i, pos;

	*argc = 0;
	*argv = NULL;

	arg = (char *)malloc(LINE_LENGTH);
	if (arg == NULL)
		return ENOMEM;
	bzero(arg, LINE_LENGTH);

	/* Get argc first */
	for (pos = 0, *argc = 0;; (*argc)++) {
		nextarg(cmd, &pos, arg);
		if (strlen(arg) == 0)
			break;
		bzero(arg, LINE_LENGTH);
	}
	free(arg);

	if (*argc == 0)
		return EINVAL;
	argvlen = PAGE_SIZE + *argc * LINE_LENGTH;

	/* Allocate and initialize argv array */
	ptr = (char *)malloc(argvlen);
	if (ptr == NULL) {
		*argc = 0;
		return ENOMEM;
	}
	bzero(ptr, argvlen);

	*argv = (char **)ptr;
	for (i = 0; i < *argc; i++)
		(*argv)[i] = ptr + PAGE_SIZE + i * LINE_LENGTH;
#if _DEBUG
	printf("argv_alloc: argc %d argv %08x\n", *argc, (u_int) * argv);
#endif

	/* Copy argv strings */
	for (pos = 0, i = 0; i < *argc; i++)
		nextarg(cmd, &pos, (*argv)[i]);
#if _DEBUG
	for (pos = 0, i = 0; i < *argc; i++)
		printf("argv_alloc: %s\n", (*argv)[i]);
#endif
	return 0;
}

void argv_free(int argc, char **argv)
{
#if _DEBUG
	printf("argv_free: argc %d argv %08x\n", argc, (u_int) argv);
#endif
	free(argv);
}
