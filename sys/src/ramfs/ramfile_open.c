#include <dev.h>
#include <errno.h>
#include <fcntl.h>
#include <fs/ramfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

int ramfile_open(file_t file)
{
	ramfile_t ramfile;
	int bufsize, ramfileno, result;

	/* Grab an open ramfiletab slot */
	mutex_lock(&ramfiletabmutex);

	for (ramfileno = 0;
	     ramfileno < RAM_FILES &&
	     ramfiletab[ramfileno].flags & RAMF_INUSE; ramfileno++) ;
	if (ramfileno == RAM_FILES) {
#if _DEBUG
		kprintf("ramfile_open: ramfiletab full\n");
#endif
		mutex_unlock(&ramfiletabmutex);
		return EAGAIN;
	}
	ramfile = (ramfile_t) & (ramfiletab[ramfileno]);
	ramfile->flags |= RAMF_INUSE;
	mutex_unlock(&ramfiletabmutex);

	dev_ioctl(file->fs->devno, GET_BUFFER_SIZE, &bufsize);
	file->data = ramfile;
	file->bufsize = bufsize;

	if (file->flags & O_RDONLY) {
		struct seek seekargs;

		result = ramfs_lookup(file, file->path);
		if (result < 0)
			goto openerror;

		/* Lock device */
		result = dev_ioctl(file->fs->devno, LOCK, NULL);
		if (result < 0) {
#if _DEBUG
			kprintf("ramfile_open: device lock failed (%s)\n",
				strerror(result));
#endif
			goto openerror;
		}
		/* Seek to device position */
#if _DEBUG
		kprintf("ramfile_open: blkno %d\n", ramfile->blkno);
#endif
		seekargs.offset = ramfile->blkno;
		seekargs.whence = SEEK_SET;
		result = dev_ioctl(file->fs->devno, SEEK_BLOCK, &seekargs);
		if (result < 0) {
#if _DEBUG
			kprintf("ramfile_open: device seek failed (%s)\n",
				strerror(result));
#endif
			goto openerror;
		}

		/* Get buffer for first cluster */
		if (file->buf != NULL)
			brel(file->buf);
		file->buf = bget(file->bufsize);
		blen(file->buf) = file->bufsize;

		result = dev_read(file->fs->devno, &(file->buf));
		if (result < 0) {
#if _DEBUG
			kprintf("ramfile_open: read failed (%s)\n",
				strerror(result));
#endif
			brel(file->buf);
			dev_ioctl(file->fs->devno, UNLOCK, NULL);
			goto openerror;
		}
		dev_ioctl(file->fs->devno, UNLOCK, NULL);
	}
	return 0;

 openerror:
	mutex_lock(&ramfiletabmutex);
	ramfile_clear(ramfile);
	mutex_unlock(&ramfiletabmutex);
	return result;
}
