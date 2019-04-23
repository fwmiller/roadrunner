#include <dev.h>
#include <errno.h>
#include <fs/ramfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

ramfs_direntry_t rootdir;
int rootdir_entries = 0;

int ramfiles_pos = 0;

static int ramfs_rootdir_read(fs_t fs);

int ramfs_mount(fs_t fs)
{
	ramfs_rootdir_read(fs);
	/* ramfiles_pos now points at the beginning of the files area */

	ramfs_rootdir_dump();

	return 0;
}

/*
 * This routine will create and populate an in-memory version of the root
 * directory found on the ramdisk device
 */
static int
ramfs_rootdir_read(fs_t fs)
{
	buf_t b;
	unsigned char *buf;
	int bufsize, i, rootdirsize;
	int result;

	dev_ioctl(fs->devno, GET_BUFFER_SIZE, &bufsize);
#if _DEBUG
	kprintf("ramfs_rootdir_read: bufsize = %d\n", bufsize);
#endif
	b = bget(bufsize);
	blen(b) = bufsize;

	/* Get length of rootdir */
	buf = (unsigned char *) malloc(bufsize);
	if (buf == NULL) {
		bfree(b);
		return (-1);
	}
	result = dev_read(fs->devno, &b);
	if (result < 0) {
		free(buf);
		bfree(b);
		return (-1);
	}
	for (i = 0; i < bufsize; i++) {
		if (*(bstart(b) + bpos(b)) == '\n')
			break;

		buf[i] = *(bstart(b) + bpos(b));
		bpos(b) += 1;
	}
	rootdirsize = atoi((const char *)buf);
#if _DEBUG
	kprintf("ramfs_mount: rootdirsize = %d\n", rootdirsize);
#endif

	return 0;
#if 0
	buf_t b;
	unsigned char buf[DENAME_LEN];
	int bufsize, i, j, offset, result;

	dev_ioctl(fs->devno, GET_BUFFER_SIZE, &bufsize);
#if _DEBUG
	kprintf("ramfs_rootdir_read: bufsize = %d\n", bufsize);
#endif
	b = bget(bufsize);
	blen(b) = bufsize;

	/* Buffer to store strings temporarily */
	memset(buf, 0, DENAME_LEN);

	dev_ioctl(fs->devno, LOCK, NULL);

	/* Get the number of file entries in the root directory */
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
	rootdir_entries = atoi((const char *)buf);
	kprintf("ramfs_mount: %d files\n", rootdir_entries);

	/* Allocate in-memory root directory to hold the entries */
	rootdir = (ramfs_direntry_t)
		malloc(rootdir_entries * sizeof(struct ramfs_direntry));
	if (rootdir == NULL) {
		dev_ioctl(fs->devno, UNLOCK, NULL);
		return (-1);
	}
	memset(rootdir, 0, rootdir_entries * sizeof(struct ramfs_direntry));

	for (i = 0, offset = 0; i < rootdir_entries; i++) {
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
		strcpy(rootdir[i].name, (const char *)buf);

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
		rootdir[i].size = atoi((const char *)buf);

		rootdir[i].offset = offset;
		offset += rootdir[i].size;
	}
	dev_ioctl(fs->devno, UNLOCK, NULL);

	return 0;
#endif
}
