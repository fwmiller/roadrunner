#include <fs/ramfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DENAME_LEN	80

struct direntry {
	char name[DENAME_LEN];
	u_long size;
};

typedef struct direntry *direntry_t;

static direntry_t dir;
static int pos = 0;

extern unsigned char ___bin_ramfs[];

int ramfs_init()
{
	unsigned char buf[DENAME_LEN];
	int i, j;
	int entries;

	memset(buf, 0, DENAME_LEN);

	/* Get number of file entries */
	for (i = 0;; i++, pos++) {
		if (___bin_ramfs[pos] == '\n') {
			pos++;
			break;
		}
		buf[i] = ___bin_ramfs[pos];
	}
	entries = atoi((const char *)buf);

	/* Allocate a file table to hold the entries */
	dir = (direntry_t) malloc(entries * sizeof(struct direntry));
	if (dir == NULL)
		return (-1);

	memset(dir, 0, entries * sizeof(struct direntry));

	for (i = 0; i < entries; i++) {
		memset(buf, 0, DENAME_LEN);
		for (j = 0;; j++, pos++) {
			if (___bin_ramfs[pos] == ',') {
				pos++;
				break;
			}
			buf[j] = ___bin_ramfs[pos];
		}
		strcpy(dir[i].name, (const char *)buf);

		memset(buf, 0, DENAME_LEN);
		for (j = 0;; j++, pos++) {
			if (___bin_ramfs[pos] == '\n') {
				pos++;
				break;
			}
			buf[j] = ___bin_ramfs[pos];
		}
		dir[i].size = atoi((const char *)buf);
	}

	/* Dump the file table entries */
	for (i = 0; i < entries; i++)
		kprintf("%s %u\n", dir[i].name, dir[i].size);

	return 0;
}
