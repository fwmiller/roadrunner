#include <dev.h>
#include <errno.h>
#include <fs/ramfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

ramfs_direntry_t dir;
int ramfiles_pos = 0;
struct ramfile ramfiletab[RAM_FILES];
struct mutex ramfiletabmutex;

int
ramfs_mount(fs_t fs)
{
	buf_t b;
	unsigned char buf[DENAME_LEN];
	int i, j;
	int entries;
	int result;

	b = bget(RAMFILE_BUFSIZE);
	blen(b) = RAMFILE_BUFSIZE;

	memset(buf, 0, DENAME_LEN);

	dev_ioctl(fs->devno, LOCK, NULL);

	for (i = 0;; i++, ramfiles_pos++) {
		result = dev_read(fs->devno, &b);
		if (result < 0) {
			dev_ioctl(fs->devno, UNLOCK, NULL);
			return result;
		}
		if (*(bstart(b)) == '\n') {
			ramfiles_pos++;
			break;
		}
		buf[i] = *(bstart(b));
	}
	dev_ioctl(fs->devno, UNLOCK, NULL);

	entries = atoi((const char *)buf);
	kprintf("ramfs_mount: %d files\n", entries);

	/* Allocate a file table to hold the entries */
	dir = (ramfs_direntry_t)
		malloc(entries * sizeof(struct ramfs_direntry));
	if (dir == NULL)
		return (-1);

	memset(dir, 0, entries * sizeof(struct ramfs_direntry));

#if 0
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
#endif
	return ENOSYS;
}
