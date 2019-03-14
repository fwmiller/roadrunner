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
#include <errno.h>
#include <fcntl.h>
#include <fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys.h>
#include <sys/argv.h>
#include <sys/proc.h>

#define CMDLINE_LENGTH		132
#define PROMPT			">"

void subst(char *cmdline, int *argc, char ***argv);

static int get_cmdline(char *cmdline, int len)
{
	int pos;
	char ch;

	bzero(cmdline, len);
	for (pos = 0;;) {
		if ((ch = getchar()) == 0) {
#if _DEBUG
			printf("get_cmdline: getchar returned zero\n");
#endif
			return EFILEEOF;
		}
		if (!isprint(ch) && ch != '\r' && ch != '\n' && ch != '\b') {
#if _DEBUG
			printf("get_cmdline: unrecognized character (%02x)\n",
			       ch);
#endif
			continue;
		}
		if (ch == '\n' || ch == '\r') {
			printf("\n");
			break;
		}
		if (ch == '\b' && pos > 0) {
			cmdline[--pos] = '\0';
			printf("\b \b");
		} else if (ch != '\b' && ch != '\r') {
			cmdline[pos++] = ch;
			printf("%c", ch);
			if (pos == LINE_LENGTH - 1)
				break;
		}
	}
	return pos;
}

int main(int argc, char **argv)
{
	char cmdline[LINE_LENGTH], path[PATH_LENGTH], tmp[PATH_LENGTH];
	int argc1;
	char **argv1;
	char *cmd;
	pid_t pid;
	int i, join, len, result;

	/* Process command line arguments */
	for (i = 1; i < argc; i++)
		if (strcmp(argv[i], "-stdin") == 0 && i + 1 < argc) {
			int fd = open(argv[i + 1], O_RDONLY);

			if (fd < 0)
				continue;
			close(getstdpath(getpid(), PFD_STDIN));
			setstdpath(getpid(), PFD_STDIN, fd);

		} else if (strcmp(argv[i], "-stdout") == 0 && i + 1 < argc) {
			int fd = open(argv[i + 1], O_WRONLY);

			if (fd < 0)
				continue;
			close(getstdpath(getpid(), PFD_STDOUT));
			setstdpath(getpid(), PFD_STDOUT, fd);

		} else if (strcmp(argv[i], "-stderr") == 0 && i + 1 < argc) {
			int fd = open(argv[i + 1], O_WRONLY);

			if (fd < 0)
				continue;
			close(getstdpath(getpid(), PFD_STDERR));
			setstdpath(getpid(), PFD_STDERR, fd);
		}

	/* Main command processing loop */
	for (;;) {
		getcwd(path, PATH_LENGTH);
		printf("%s%s ", path, PROMPT);
		len = get_cmdline(cmdline, CMDLINE_LENGTH);
		if (len <= 0)
			continue;

		argc1 = 0;
		argv1 = NULL;
		result = argv_alloc(cmdline, &argc1, &argv1);
		if (result < 0 || argc1 == 0 || argv1 == NULL) {
			printf("could not allocate argv1 array (%s)\n",
			       strerror(result));
			continue;
		}
		if (argc1 > 1 && strcmp(argv1[argc1 - 1], "&") == 0)
			join = 0;
		else
			join = 1;

		/* Perform filename substitutions */
		if (argc1 > 0 && argv1 != NULL)
			subst(cmdline, &argc1, &argv1);
#if _DEBUG
		printf("sh: cmdline %s\n", cmdline);
#endif

		/* Built-in commands */
		if (strcmp(argv1[0], "cd") == 0) {
			if (argc1 < 2) {
				printf("missing directory name\n");
				continue;
			}
			if (argc1 > 2) {
				printf("too many arguments\n");
				continue;
			}
			result = chdir(argv1[1]);
			if (result < 0)
				printf("%s\n", strerror(result));
			continue;
		}
		if (strcmp(argv1[0], "exit") == 0)
			return 0;

		if (strcmp(argv1[0], "h") == 0 || strcmp(argv1[0], "help") == 0) {

#define CMDLIST "cd exit h help reboot"

			printf("built-in commands: %s\n", CMDLIST);
			continue;
		}
		if (strcmp(argv1[0], "reboot") == 0)
			reboot();

		/* Load command program in a new process */
		cmd = argv1[0];
		if (cmd[0] != '/') {
			strcpy(tmp, cmd);
			mkpath("/bin", tmp, cmd);
		}
#if _DEBUG
		printf("sh: cmd %s\n", cmd);
#endif
		pid = exec(cmd, argc1, argv1);
		if (pid < 0) {
			printf("%s\n", strerror(pid));
			continue;
		}
#if _DEBUG
		printf("sh: pid %d\n", pid);
#endif
		if (join)
			wait(pid);
	}
}
