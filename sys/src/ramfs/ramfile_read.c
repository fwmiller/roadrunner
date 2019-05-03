#include <dev.h>
#include <errno.h>
#include <fs/ramfs.h>
#if _DEBUG
#include <stdio.h>
#endif
#include <stdlib.h>
#if _DEBUG
#include <string.h>
#endif
#include <sys/ioctl.h>

int
ramfile_read(file_t file)
{
	ramfile_t ramfile = (ramfile_t) file->data;
	struct seek seekargs;
	int result;

	if (file->flags & F_EOF)
		return EINVAL;

	/* File blks are sequential */
	ramfile->blkno++;

	/* Assume fs layer has provided a buffer */

	result = dev_ioctl(file->fs->devno, LOCK, NULL);
	if (result < 0)
		return result;

	seekargs.offset = ramfile->blkno;
	seekargs.whence = SEEK_SET;
	result = dev_ioctl(file->fs->devno, SEEK_BLOCK, &seekargs);
	if (result < 0) {
#if _DEBUG
		kprintf("ramfile_read: device seek failed (%s)\n",
			strerror(result));
#endif
		dev_ioctl(file->fs->devno, UNLOCK, NULL);
		return result;
	}
	result = dev_read(file->fs->devno, &(file->buf));
	if (result < 0) {
#if _DEBUG
		kprintf("ramfile_read: read failed (%s)\n",
			strerror(result));
#endif
	}
	dev_ioctl(file->fs->devno, UNLOCK, NULL);
	return result;
}
