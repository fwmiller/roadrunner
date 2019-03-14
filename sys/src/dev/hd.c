#include <dev.h>
#include <dev/hd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/config.h>
#include <sys/ioctl.h>

#define HDS	16

static int nexthd = 0;

struct hd hdtab[HDS];

void hdtab_init()
{
	bzero(hdtab, HDS * sizeof(struct hd));
}

int hd_alloc(hd_t hd)
{
	struct dev_ops ops;
	char name[8];
	int result;

	if (nexthd == HDS)
		return EAGAIN;

	bcopy(hd, &(hdtab[nexthd]), sizeof(struct hd));

	ops.init = NULL;
	ops.shut = NULL;
	ops.ioctl = hd_ioctl;
	ops.specific.blk_ops.read = hd_read;
	ops.specific.blk_ops.write = hd_write;

	sprintf(name, "hd%d", nexthd);
	result = dev_inst(name, DEV_TYPE_BLK, &ops, &(hdtab[nexthd]));
	if (result < 0)
		kprintf("hd_alloc: device install failed (%s)\n",
			strerror(result));

	nexthd++;
	return 0;
}

int hd_ioctl(void *dev, int cmd, void *args)
{
	hd_t hd = (hd_t) dev;

	if (hd->ioctl == NULL)
		return ENOSYS;

	return (*(hd->ioctl)) (hd->part, cmd, args);
}

int hd_read(void *dev, buf_t * b)
{
	hd_t hd = (hd_t) dev;

	if (hd->read == NULL)
		return ENOSYS;

	return (*(hd->read)) (hd->part, b);
}

int hd_write(void *dev, buf_t * b)
{
	hd_t hd = (hd_t) dev;

	if (hd->read == NULL)
		return ENOSYS;

	return (*(hd->write)) (hd->part, b);
}
