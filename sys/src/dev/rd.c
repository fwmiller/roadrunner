#include <dev/rd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mutex.h>
#include "ramdisk.c"

#define RDBLKSIZE	512

extern unsigned char ___bin_ramdisk[];

static struct mutex rdmutex;
static int rdblkno = 0;

void bufdump(char *buf, int size);

int rd_init(void *dev)
{
	kprintf("rd: %d bytes\n", sizeof(___bin_ramdisk));

	mutex_clear(&rdmutex);

	return 0;
}

int rd_shut(void *dev)
{
	return 0;
}

int rd_ioctl(void *dev, int cmd, void *args)
{
	switch (cmd) {
	case LOCK:
		return mutex_lock(&rdmutex);

	case UNLOCK:
		return mutex_unlock(&rdmutex);

	case GET_BUFFER_SIZE:
		if (args == NULL)
			return EINVAL;

		*((u_long *) args) = RDBLKSIZE;
		return 0;

	case SEEK_BLOCK:
		{
			struct seek *seekargs;
			int blkno;

			if (args == NULL)
				return EINVAL;

			seekargs = (struct seek *) args;
			blkno = (int)seekargs->offset;
			if (blkno < 0 ||
			    (blkno * RDBLKSIZE) >= sizeof(___bin_ramdisk))
				return EINVAL;

			rdblkno = blkno;
			return 0;
		}
	}
	return ENOTTY;
}

int rd_read(void *dev, buf_t * b)
{
	if (b == NULL || *b == NULL)
		return EINVAL;

	if (rdblkno < 0 || (rdblkno * RDBLKSIZE) >= sizeof(___bin_ramdisk))
		return EINVAL;

	bcopy(___bin_ramdisk + (rdblkno * RDBLKSIZE), bstart(*b), RDBLKSIZE);
	blen(*b) = RDBLKSIZE;

	rdblkno++;

	return 0;
}

int rd_write(void *dev, buf_t * b)
{
	return ENOSYS;
}
