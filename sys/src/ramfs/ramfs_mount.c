#include <dev.h>
#include <errno.h>
#include <fs/ramfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys.h>
#include <sys/ioctl.h>

ramfs_direntry_t rootdir;
int rootdir_entries = 0;
int ramfiles_blkno = 0;

static int ramfs_rootdir_read(fs_t fs);

int ramfs_mount(fs_t fs)
{
	ramfs_rootdir_read(fs);
	/* ramfile_pos now points at the beginning of the files area */

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
	unsigned char *buf, sizebuf[32];
	int bufsize, i, j, pos, offset, rootdirsize;
	int result;

	dev_ioctl(fs->devno, LOCK, NULL);
	dev_ioctl(fs->devno, GET_BUFFER_SIZE, &bufsize);
#if _DEBUG
	kprintf("ramfs_rootdir_read: bufsize = %d\n", bufsize);
#endif
	b = bget(bufsize);
	blen(b) = bufsize;

	/* Get length of rootdir */
	result = dev_read(fs->devno, &b);
	if (result < 0) {
		bfree(b);
		return (-1);
	}
	rootdirsize = atoi((const char *)bstart(b));
#if _DEBUG
	kprintf("ramfs_rootdir_read: rootdirsize = %d\n", rootdirsize);
#endif

	/* Read the rootdir from the ramdisk */
	buf = (unsigned char *) malloc(ALIGN(rootdirsize, bufsize));
	if (buf == NULL) {
		bfree(b);
		return (-1);
	}
	for (ramfiles_blkno = 1, i = 0;
	     i < (rootdirsize / bufsize) + 1;
	     i++, ramfiles_blkno++) {
		result = dev_read(fs->devno, &b);
		if (result < 0) {
			free(buf);
			bfree(b);
			return (-1);
		}
		bcopy(bstart(b), buf + (i * bufsize), bufsize);
	}
	bfree(b);
	dev_ioctl(fs->devno, UNLOCK, NULL);

	/* Count rootdir entries */
	for (i = 0; i < bufsize; i++) {
		if (buf[i] == 0)
			break;

		if (buf[i] == '\n')
			rootdir_entries++;
	}
#if _DEBUG
	kprintf("ramfs_rootdir_read: rootdir_entries = %d\n",
		rootdir_entries);
#endif
	/* Allocate in-memory structured root directory */
	rootdir = (ramfs_direntry_t)
		malloc(rootdir_entries * sizeof(struct ramfs_direntry));
	if (rootdir == NULL) {
		free(buf);
		return (-1);
	}
	memset(rootdir, 0, rootdir_entries * sizeof(struct ramfs_direntry));

	/* Loop over the directory entries */
	for (i = 0, pos = 0, offset = 0; i < rootdir_entries; i++) {
		/* Retrieve file name */
		for (j = 0;; j++, pos++) {
			if (buf[pos] == ',') {
				pos++;
				break;
			}
			rootdir[i].name[j] = buf[pos];
		}

		/* Retrieve file size */
		memset(sizebuf, 0, 32);
		for (j = 0;; j++, pos++) {
			if (buf[pos] == '\n') {
				pos++;
				break;
			}
			sizebuf[j] = buf[pos];
		}
		rootdir[i].size = atoi((const char *)sizebuf);

		/* Set file offset */
		rootdir[i].offset = offset;
		offset += rootdir[i].size;
	}
	free(buf);
	return 0;
}
