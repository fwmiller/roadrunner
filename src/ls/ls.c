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

/* Group member */
struct gm {
	struct gm *next;
	char *entry;
};

#define CMP_LT		0
#define CMP_NE		1

#define CMP_NUM(OP, TYPE, A, B)						\
{									\
    if (OP == CMP_LT) {							\
	if (*((TYPE *) (A)) < *((TYPE *) (B)))				\
	    return 1;							\
    } else if (OP == CMP_NE) {						\
	if (*((TYPE *) (A)) != *((TYPE *) (B)))				\
	    return 1;							\
    }									\
}

#if _DEBUG
#define GROUP_CNT							\
{									\
    struct gm *t;							\
    int cnt;								\
    for (cnt = 0, t = g; t != NULL; cnt++, t = t->next);		\
    printf("ls: cnt %d\n", cnt);					\
}
#endif

static int cmp(int op, int type, void *gm1, void *gm2)
{
	if (type == ATTR_CHAR) {
		CMP_NUM(op, char, gm1, gm2);
	} else if (type == ATTR_SHORT) {
		CMP_NUM(op, short, gm1, gm2);
	} else if (type == ATTR_INT) {
		CMP_NUM(op, int, gm1, gm2);
	} else if (type == ATTR_UCHAR) {
		CMP_NUM(op, u_char, gm1, gm2);
	} else if (type == ATTR_USHORT) {
		CMP_NUM(op, u_short, gm1, gm2);
	} else if (type == ATTR_UINT) {
		CMP_NUM(op, u_int, gm1, gm2);
	} else if (type == ATTR_STRING) {
		if (op == CMP_LT) {
			if (strcmp((char *)gm1, (char *)gm2) < 0)
				return 1;
		} else if (op == CMP_NE) {
			if (strcmp((char *)gm1, (char *)gm2) != 0)
				return 1;
		}
	}
	return 0;
}

static void sort(struct gm **g, int type, int pos)
{
	struct gm *gm, *p, *n, *s;

	for (s = NULL; *g != NULL;) {
		gm = *g;
		*g = gm->next;
		gm->next = NULL;

		for (p = NULL, n = s; n != NULL; p = n, n = n->next)
			if (cmp(CMP_LT, type, gm->entry + pos, n->entry + pos))
				break;

		if (n == NULL) {
			if (p == NULL)
				s = gm;
			else
				p->next = gm;
		} else {
			if (p == NULL) {
				gm->next = s;
				s = gm;
			} else {
				gm->next = n;
				p->next = gm;
			}
		}
	}
	*g = s;
}

static void dup(struct gm *g, int type, int pos)
{
	for (; g != NULL; g = g->next) {
		while (g->next != NULL) {
			if (cmp
			    (CMP_NE, type, g->entry + pos,
			     g->next->entry + pos))
				break;
			else {
				struct gm *t = g->next;

				g->next = t->next;
				free(t->entry);
				free(t);
			}
		}
		if (g->next == NULL)
			break;
	}
}

#define ATTR_CHAR_LEN		4
#define ATTR_SHORT_LEN		6
#define ATTR_INT_LEN		10

static void disp(attrlist_t l, struct gm *g, int pos)
{
	struct gm *gm;
	struct gm **gc;
	int wid, len, cols, rows, i, j, k, m;

	wid = COLS;

	if (l->attr[l->key]->type == ATTR_CHAR) {
		len = ATTR_CHAR_LEN;
	} else if (l->attr[l->key]->type == ATTR_SHORT) {
		len = ATTR_SHORT_LEN;
	} else if (l->attr[l->key]->type == ATTR_INT) {
		len = ATTR_INT_LEN;
	} else if (l->attr[l->key]->type == ATTR_UCHAR) {
		len = ATTR_CHAR_LEN;
	} else if (l->attr[l->key]->type == ATTR_USHORT) {
		len = ATTR_SHORT_LEN;
	} else if (l->attr[l->key]->type == ATTR_UINT) {
		len = ATTR_INT_LEN;
	} else if (l->attr[l->key]->type == ATTR_STRING) {
		for (len = 0, gm = g; gm != NULL; gm = gm->next) {
			for (i = 0;
			     i < l->attr[l->key]->len
			     && *(gm->entry + pos + i) != '\0'; i++) ;
			if (i > len)
				len = i;
		}
	}
	for (i = 0, gm = g; gm != NULL; i++, gm = gm->next) ;
	if (i == 0)
		return;
	cols = (wid - 1) / (len + 2);
	cols = (cols == 0 ? 1 : cols);
	rows = (i / cols) + (i % cols > 0 ? 1 : 0);
	rows = (rows == 0 ? 1 : rows);

#if _DEBUG
	printf("disp: i %d len %d rows %d cols %d\n", i, len, rows, cols);
#endif
	gc = (struct gm **)malloc(cols * sizeof(struct gm *));
	if (gc == NULL)
		return;
	for (i = 0; i < cols; i++)
		gc[i] = NULL;

	for (gm = g, i = 0; i < cols && gm != NULL; i++) {
		gc[i] = gm;
		for (j = 0; j < rows && gm != NULL; j++)
			gm = gm->next;
	}
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++)
			if (gc[j] != NULL) {
				if (l->attr[l->key]->type == ATTR_CHAR) {
					printf("%4d  ",
					       (int)*(gc[j]->entry + pos));
				} else if (l->attr[l->key]->type == ATTR_SHORT) {
					printf("%6d  ", (int)
					       *((short *)
						 (gc[j]->entry + pos)));
				} else if (l->attr[l->key]->type == ATTR_INT) {
					printf("%10d  ", *((int *)
							   (gc[j]->entry +
							    pos)));
				} else if (l->attr[l->key]->type == ATTR_UCHAR) {
					printf("%4u  ",
					       (u_int) *
					       ((u_char *) (gc[j]->entry +
							    pos)));
				} else if (l->attr[l->key]->type == ATTR_USHORT) {
					printf("%6u  ",
					       (u_int) *
					       ((u_short *) (gc[j]->entry +
							     pos)));
				} else if (l->attr[l->key]->type == ATTR_UINT) {
					printf("%10u  ",
					       (u_int) *
					       ((u_int *) (gc[j]->entry +
							   pos)));
				} else if (l->attr[l->key]->type == ATTR_STRING) {
					for (m = 0;
					     m < l->attr[l->key]->len
					     && *(gc[j]->entry + pos + m) !=
					     '\0'; m++) ;
					for (k = 0; k < m; k++)
						printf("%c",
						       *(gc[j]->entry + pos +
							 k));
					for (; k < len; k++)
						printf("%c", ' ');
					printf("  ");
				}
				gc[j] = gc[j]->next;
			}
		printf("\n");
	}
	free(gc);
}

void displ(attrlist_t l, struct gm *g)
{
	struct gm *gm;
	int *len;
	int i, j, pos;

	/* Compute the maximum length of each string field */
	len = (int *)malloc(l->n * sizeof(int));

	if (len == NULL)
		return;
	for (pos = 0, i = 0; i < l->n; i++) {
		if (l->attr[i]->type == ATTR_STRING) {
			for (len[i] = 0, gm = g; gm != NULL; gm = gm->next) {
				for (j = 0;
				     j < l->attr[i]->len &&
				     *(gm->entry + pos + j) != '\0'; j++) ;
				if (j > len[i])
					len[i] = j;
			}
		}
		pos += l->attr[i]->len;
	}

	for (gm = g; gm != NULL; gm = gm->next) {
		for (pos = 0, i = 0; i < l->n; i++) {
			if (l->attr[i]->type == ATTR_CHAR) {
				printf("%4d  ",
				       (int)*((char *)(gm->entry + pos)));
				pos += sizeof(char);
			} else if (l->attr[i]->type == ATTR_SHORT) {
				printf("%6d  ", (int)
				       *((short *)(gm->entry + pos)));
				pos += sizeof(short);
			} else if (l->attr[i]->type == ATTR_INT) {
				printf("%10d  ", *((int *)(gm->entry + pos)));
				pos += sizeof(int);
			} else if (l->attr[i]->type == ATTR_UCHAR) {
				printf("%4u  ",
				       (u_int) *
				       ((u_char *) (gm->entry + pos)));
				pos += sizeof(u_char);
			} else if (l->attr[i]->type == ATTR_USHORT) {
				printf("%6u  ",
				       (u_int) *
				       ((u_short *) (gm->entry + pos)));
				pos += sizeof(u_short);
			} else if (l->attr[i]->type == ATTR_UINT) {
				printf("%10u  ",
				       *((u_int *) (gm->entry + pos)));
				pos += sizeof(u_int);
			} else if (l->attr[i]->type == ATTR_STRING) {
				int slen;

				slen = strlen(gm->entry + pos);
				for (j = 0; j < len[i]; j++)
					if (j < slen)
						printf("%c",
						       *(gm->entry + pos + j));
					else
						printf(" ");
				printf("  ");
				pos += l->attr[i]->len;
			}
		}
		printf("\n");
	}
	free(len);
}

int main(int argc, char **argv)
{
	char *cwd, *dir;
	struct attrlist l;
	struct gm *g, *gm;
	fsrectab_t fstab;
	fsrec_t fsrec;
	void *ptr;
	int displong = 0;
	int fd, size, pos, i, j, result;

	cwd = (char *)malloc(PATH_LENGTH);
	if (cwd == NULL) {
		printf("could not allocate cwd buffer\n");
		return ENOMEM;
	}
	bzero(cwd, PATH_LENGTH);

	dir = (char *)malloc(PATH_LENGTH);
	if (dir == NULL) {
		printf("could not allocate dir buffer\n");
		free(cwd);
		return ENOMEM;
	}
	bzero(dir, PATH_LENGTH);

	/* Command line arguments */
	for (getcwd(dir, PATH_LENGTH), i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-l") == 0)
			displong = 1;
		else
			strcpy(dir, argv[i]);
	}
	/* Check whether specified path is really a directory */
	getcwd(cwd, PATH_LENGTH);
	result = chdir(dir);
	if (result < 0) {
		printf("%s\n", strerror(result));
		free(dir);
		free(cwd);
		return result;
	}
	chdir(cwd);
	free(cwd);

#if _DEBUG
	printf("ls: open %s\n", dir);
#endif
	/* Open directory to be listed */
	if ((fd = open(dir, O_RDONLY)) < 0) {
		printf("could not open %s\n", dir);
		free(dir);
		return fd;
	}
	/* Get directory member attributes */
	if ((result = attr(fd, &l)) < 0) {
		printf("could not get attributes\n");
		close(fd);
		free(dir);
		return result;
	}
	/* Determine the size of each directory member */
	for (size = 0, i = 0; i < l.n; i++)
		size += l.attr[i]->len;
#if _DEBUG
	printf("ls: directory member size %d\n", size);
#endif

	/* Build list of directory entries */
	for (g = NULL, gm = NULL;; gm = NULL) {
		gm = (struct gm *)malloc(sizeof(struct gm));
		if (gm == NULL)
			goto lscleanup;

		gm->next = NULL;
		gm->entry = NULL;

		ptr = malloc(size);
		gm->entry = (char *)ptr;
		if (gm->entry == NULL) {
			printf("could not allocate directory member buffer\n");
			free(gm);
			goto lscleanup;
		}
		bzero(gm->entry, size);

		if ((result = readdir(fd, gm->entry)) < 0) {
			if (result == EFILEEOF) {
#if _DEBUG
				printf("ls: eof\n");
#endif
				free(gm->entry);
				free(gm);
				break;
			}
#if _DEBUG
			printf("ls: readdir failed (%s)\n", strerror(result));
#endif
			free(gm->entry);
			free(gm);
			goto lscleanup;
		}
		gm->next = g;
		g = gm;
	}

	/* Compute position of key attribute in entry */
	for (pos = 0, j = 0; j < l.key; j++)
		pos += l.attr[j]->len;
#if _DEBUG
	GROUP_CNT;
	disp(&l, g, pos);
#endif

	/* If key attribute type is string then add elements from fstab */
	if (l.attr[l.key]->type == ATTR_STRING) {
		int len;

		result = getfstab(&fstab);
		if (result < 0)
			goto lscleanup;

		if (fstab == NULL) {
#if _DEBUG
			printf("ls: fstab not found\n");
#endif
		} else {
#if _DEBUG
			printf("ls: fstab entries %d\n", fstab->entries);
#endif
			for (i = 0; i < fstab->entries; i++) {
				len = strlen(dir);
				fsrec =
				    (fsrec_t) (fstab->recs + i * FSREC_SIZE);
#if _DEBUG
				printf("ls: len %d path %s\n", len,
				       fsrec->path);
#endif
				if (len < strlen(fsrec->path) &&
				    memcmp(fsrec->path, dir, len) == 0) {

					int start, end;

					gm = (struct gm *)
					    malloc(sizeof(struct gm));
					if (gm == NULL)
						goto lscleanup;

					gm->next = NULL;
					ptr = malloc(size);
					gm->entry = (char *)ptr;
					if (gm->entry == NULL) {
						free(gm);
						goto lscleanup;
					}
					bzero(gm->entry, size);

					start = strlen(dir);
					for (end = start;
					     fsrec->path[end] != '/'
					     && fsrec->path[end] != '\0';
					     end++) ;

					len = end - start;
					strncpy(gm->entry + pos,
						fsrec->path + start,
						(len <
						 ATTR_NAME_LEN ? len :
						 ATTR_NAME_LEN));
					gm->next = g;
					g = gm;
				}
			}
		}
	}
#if _DEBUG
	GROUP_CNT;
	disp(&l, g, pos);
#endif
	/* Remove duplicates */
	dup(g, l.attr[l.key]->type, pos);
#if _DEBUG
	GROUP_CNT;
#endif
	/* Sort */
	sort(&g, l.attr[l.key]->type, pos);
#if _DEBUG
	GROUP_CNT;
#endif
	/* Display list */
	if (displong) {
		displ(&l, g);
	} else {
		for (pos = 0, i = 0; i != l.key; pos += l.attr[i]->len, i++) ;
		disp(&l, g, pos);
	}

 lscleanup:
	/* Clean up directory member data */
	for (gm = g; gm != NULL; gm = g) {
		g = g->next;
		free(gm->entry);
		free(gm);
	}

	/* Clean up directory member attribute data */
	for (i = 0; i < l.n; i++)
		free(l.attr[i]);
	free(l.attr);
	close(fd);
	free(dir);
	return 0;
}
