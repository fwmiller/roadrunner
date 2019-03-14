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

#include <fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/argv.h>
#include <sys/config.h>
#include <unistd.h>

int has_pattern(char *path);

int pattern_break(char *path, int pos, char **prefix,
		  char **pattern, char **suffix);

int glob(char *pattern, char *path, char **pathv);

void subst(char *cmdline, int *argc, char ***argv)
{
	int i, j, pos, result;

	if (argv == NULL || *argv == NULL)
		return;

#if _DEBUG
	printf("subst: argc %d\n", *argc);
#endif
	/* Perform filename substitutions */
	for (i = 1; i < *argc; i++)
		if ((pos = has_pattern((*argv)[i])) >= 0) {
			char *prefix = NULL, *pattern = NULL, *suffix = NULL;

			result = pattern_break((*argv)[i], pos,
					       &prefix, &pattern, &suffix);
			if (result == 0) {
				char *filenames;

#if _DEBUG
				printf("subst:");
				if (prefix != NULL)
					printf(" prefix %s", prefix);
				if (pattern != NULL)
					printf(" pattern %s", pattern);
				if (suffix != NULL)
					printf(" suffix %s", suffix);
				printf("\n");
#endif
				/*
				 * Glob on the path represented by the prefix and the
				 * element containing the pattern
				 */
				if (strcmp(prefix, "") == 0)
					getcwd(prefix, PATH_LENGTH);
				if (prefix[0] != '/') {
					char cwd[PATH_LENGTH], tmp[PATH_LENGTH];

					getcwd(cwd, PATH_LENGTH);
					strcpy(tmp, prefix);
					mkpath(cwd, tmp, prefix);
#if _DEBUG
					printf("subst: prefix %s\n", prefix);
#endif
				}

				filenames = NULL;
				result = glob(pattern, prefix, &filenames);
#if _DEBUG
				if (result < 0)
					printf("subst: glob failed (%s)\n",
					       strerror(result));
#endif
				if (result == 0 && filenames != NULL &&
				    strcmp(filenames, "") != 0) {
					char **argv1;
					int argc1;

					/* Reassemble each filename yielded by the glob */
					argv_alloc(filenames, &argc1, &argv1);
					for (j = 0; j < argc1; j++) {
						strcpy(pattern, argv1[j]);
						mkpath(prefix, pattern,
						       argv1[j]);
						strcpy(pattern, argv1[j]);
						mkpath(pattern, suffix,
						       argv1[j]);
					}
#if 0
					for (j = 0; j < argc1; j++)
						printf("subst: %s\n", argv1[j]);
#endif
					/* Rebuild cmdline and then *argc and *argv */
					bzero(cmdline, LINE_LENGTH);
					for (j = 0; j < *argc; j++)
						if (j == 0)
							strcpy(cmdline,
							       (*argv)[0]);
						else if (j == i) {
							int k;

							for (k = 0; k < argc1;
							     k++) {
								strcat(cmdline,
								       " ");
								strcat(cmdline,
								       argv1
								       [k]);
							}
						} else {
							strcat(cmdline, " ");
							strcat(cmdline,
							       (*argv)[j]);
						}
					argv_free(argc1, argv1);
					i = 1;

				} else {
					/* Remove pattern from command line */
					for (j = 0; j < *argc; j++)
						if (j == 0)
							strcpy(cmdline,
							       (*argv)[0]);
						else if (j != i) {
							strcat(cmdline, " ");
							strcat(cmdline,
							       (*argv)[j]);
						}
				}
				if (filenames != NULL)
					free(filenames);
				argv_free(*argc, *argv);
				argv_alloc(cmdline, argc, argv);
			}
			if (suffix != NULL)
				free(suffix);
			if (pattern != NULL)
				free(pattern);
			if (prefix != NULL)
				free(prefix);
		}
}
