#include <dev/rd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mutex.h>
#include "ramdisk.c"

#define RDBUFSIZE	1

extern unsigned char ___bin_ramdisk[];

static struct mutex rdmutex;
static int rdpos = 0;

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
		*((u_long *) args) = RDBUFSIZE;
		return 0;

	case SEEK_BLOCK:
		rdpos = (int) args;
		return 0;
	}
	return ENOTTY;
}

int rd_read(void *dev, buf_t * b)
{
	return (-1);
}

int rd_write(void *dev, buf_t * b)
{
	return (-1);
}

