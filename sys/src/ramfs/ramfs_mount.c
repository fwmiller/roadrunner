#include <dev.h>
#include <errno.h>
#include <fs/ramfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

int ramfs_entries = 0;
ramfs_direntry_t dir = NULL;
int ramfiles_pos = 0;
struct ramfile ramfiletab[RAM_FILES];
struct mutex ramfiletabmutex;

int
ramfs_mount(fs_t fs)
{
	buf_t b;
	unsigned char buf[DENAME_LEN];
	int i, j;
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
	ramfs_entries = atoi((const char *)buf);
	kprintf("ramfs_mount: %d files\n", ramfs_entries);

	/* Allocate a file table to hold the entries */
	dir = (ramfs_direntry_t)
		malloc(ramfs_entries * sizeof(struct ramfs_direntry));
	if (dir == NULL) {
		dev_ioctl(fs->devno, UNLOCK, NULL);
		return (-1);
	}
	memset(dir, 0, ramfs_entries * sizeof(struct ramfs_direntry));

	for (i = 0; i < ramfs_entries; i++) {
		/* Retrieve file name */
		memset(buf, 0, DENAME_LEN);
		for (j = 0;; j++, ramfiles_pos++) {
			result = dev_read(fs->devno, &b);
			if (result < 0) {
				dev_ioctl(fs->devno, UNLOCK, NULL);
				return result;
			}
			if (*(bstart(b)) == ',') {
				ramfiles_pos++;
				break;
			}
			buf[j] = *(bstart(b));
		}
		strcpy(dir[i].name, (const char *)buf);

		/* Retrieve file size */
		memset(buf, 0, DENAME_LEN);
		for (j = 0;; j++, ramfiles_pos++) {
			result = dev_read(fs->devno, &b);
			if (result < 0) {
				dev_ioctl(fs->devno, UNLOCK, NULL);
				return result;
			}
			if (*(bstart(b)) == '\n') {
				ramfiles_pos++;
				break;
			}
			buf[j] = *(bstart(b));
		}
		dir[i].size = atoi((const char *)buf);
	}
	dev_ioctl(fs->devno, UNLOCK, NULL);

	/* ramfiles_pos now points at the beginning of the files area */

	/* Dump the file table entries */
	ramfs_file_table_dump(ramfs_entries);

	/* Clear ramfile table */
	for (i = 0; i < RAM_FILES; i++)
		ramfile_clear(&(ramfiletab[i]));

	mutex_clear(&ramfiletabmutex);
	return 0;
}
