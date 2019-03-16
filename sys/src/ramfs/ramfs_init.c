#include <fs/ramfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ramfs_direntry_t dir;
int ramfiles_pos = 0;
struct ramfile ramfiletab[RAM_FILES];
struct mutex ramfiletabmutex;

extern unsigned char ___bin_ramfs[];

int ramfs_init()
{
	unsigned char buf[DENAME_LEN];
	int i, j;
	int entries;

	memset(buf, 0, DENAME_LEN);

	/* Get number of file entries */
	for (i = 0;; i++, ramfiles_pos++) {
		if (___bin_ramfs[ramfiles_pos] == '\n') {
			ramfiles_pos++;
			break;
		}
		buf[i] = ___bin_ramfs[ramfiles_pos];
	}
	entries = atoi((const char *)buf);

	/* Allocate a file table to hold the entries */
	dir = (ramfs_direntry_t) malloc(entries * sizeof(struct ramfs_direntry));
	if (dir == NULL)
		return (-1);

	memset(dir, 0, entries * sizeof(struct ramfs_direntry));

	for (i = 0; i < entries; i++) {
		/* Retrieve file name */
		memset(buf, 0, DENAME_LEN);
		for (j = 0;; j++, ramfiles_pos++) {
			if (___bin_ramfs[ramfiles_pos] == ',') {
				ramfiles_pos++;
				break;
			}
			buf[j] = ___bin_ramfs[ramfiles_pos];
		}
		strcpy(dir[i].name, (const char *)buf);

		/* Retrieve file size */
		memset(buf, 0, DENAME_LEN);
		for (j = 0;; j++, ramfiles_pos++) {
			if (___bin_ramfs[ramfiles_pos] == '\n') {
				ramfiles_pos++;
				break;
			}
			buf[j] = ___bin_ramfs[ramfiles_pos];
		}
		dir[i].size = atoi((const char *)buf);
	}
	/* ramfiles_pos now points at the beginning of the files area */

	/* Dump the file table entries */
	for (i = 0; i < entries; i++)
		kprintf("%s %u\n", dir[i].name, dir[i].size);

	/* Clear ramfile table */
	for (i = 0; i < RAM_FILES; i++)
		ramfile_clear(&(ramfiletab[i]));

	mutex_clear(&ramfiletabmutex);

	return 0;
}
