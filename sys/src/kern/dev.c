/* 
 *  Roadrunner/pk
 *    Copyright (C) 1989-2001  Cornfed Systems, Inc.
 *
 *  The Roadrunner/pk operating system is free software; you can
 *  redistribute and/or modify it under the terms of the GNU General
 *  Public License, version 2, as published by the Free Software
 *  Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this program; if not, write to the Free
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA 02111-1307 USA
 *
 *  More information about the Roadrunner/pk operating system of
 *  which this file is a part is available on the World-Wide Web
 *  at: http://www.cornfed.com.
 *
 */

#include <errno.h>
#include <dev.h>
#include <stdlib.h>
#include <string.h>
#include <sys/config.h>
#include <sys/intr.h>

struct dev devtab[DEVICES];

void devtab_init()
{
	bzero(devtab, DEVICES * sizeof(struct dev));
}

static int valid_dev(int devno)
{
	if (devno < 0 || devno >= DEVICES)
		return 0;
	if (devtab[devno].type == DEV_TYPE_UNUSED)
		return 0;
	return 1;
}

static int char_dev(int devno)
{
	if (devno < 0 || devno >= DEVICES)
		return 0;
	if (devtab[devno].type == DEV_TYPE_CHAR)
		return 1;
	return 0;
}

static int blk_dev(int devno)
{
	if (devno < 0 || devno >= DEVICES)
		return 0;
	if (devtab[devno].type == DEV_TYPE_BLK)
		return 1;
	return 0;
}

int dev_close(int devno)
{
	disable;

	if (!valid_dev(devno) || devtab[devno].refcnt == 0) {
		enable;
		return EINVAL;
	}
	devtab[devno].refcnt--;
	enable;
	return 0;
}

int dev_get(int devno, int *c)
{
	dev_get_func_t get;

	disable;

	if (!valid_dev(devno) || !char_dev(devno)) {
		enable;
		return EINVAL;
	}
	get = DEV_GET(devno);
	enable;
	if (get == NULL)
		return ENOSYS;
	return (get) (devtab[devno].dev, c);
}

int dev_init(char *name)
{
	dev_init_func_t init;
	int i;

	if (name == NULL)
		return EINVAL;

	disable;

	for (i = 0; i < DEVICES && strcmp(devtab[i].name, name) != 0; i++) ;
	if (i == DEVICES) {
		enable;
		return EINVAL;
	}
	init = devtab[i].ops.init;
	enable;
	if (init == NULL)
		return ENOSYS;
	return (init) (devtab[i].dev);
}

int dev_inst(char *name, int type, dev_ops_t ops, void *dev)
{
	int i;

	if (name == NULL)
		return EINVAL;
	if (type < DEV_TYPE_CHAR || type > DEV_TYPE_BLK)
		return EINVAL;
	if (ops == NULL)
		return EINVAL;

	disable;

	for (i = 0; i < DEVICES && devtab[i].type != DEV_TYPE_UNUSED; i++) ;
	if (i == DEVICES) {
		enable;
		return EAGAIN;
	}
	strcpy(devtab[i].name, name);
	devtab[i].type = type;
	devtab[i].ops.init = ops->init;
	devtab[i].ops.shut = ops->shut;
	devtab[i].ops.ioctl = ops->ioctl;

	switch (type) {
	case DEV_TYPE_CHAR:
		DEV_GET(i) = ops->specific.char_ops.get;
		DEV_PUT(i) = ops->specific.char_ops.put;
		break;

	case DEV_TYPE_BLK:
		DEV_READ(i) = ops->specific.blk_ops.read;
		DEV_WRITE(i) = ops->specific.blk_ops.write;
		break;

		//default:
	}
	devtab[i].dev = dev;

	enable;
	return 0;
}

int dev_ioctl(int devno, int cmd, void *args)
{
	dev_ioctl_func_t ioctl;

	disable;

	if (!valid_dev(devno)) {
		enable;
		return EINVAL;
	}
	ioctl = devtab[devno].ops.ioctl;
	enable;
	if (ioctl == NULL)
		return ENOSYS;
	return (ioctl) (devtab[devno].dev, cmd, args);
}

int dev_open(char *name)
{
	int i;

	if (name == NULL)
		return EINVAL;

	disable;

	for (i = 0; i < DEVICES && strcmp(devtab[i].name, name) != 0; i++) ;
	if (i == DEVICES) {
		enable;
		return EAGAIN;
	}
	devtab[i].refcnt++;
	enable;
	return i;
}

int dev_put(int devno, int c)
{
	dev_put_func_t put;

	disable;

	if (!valid_dev(devno) || !char_dev(devno)) {
		enable;
		return EINVAL;
	}
	put = DEV_PUT(devno);
	enable;
	if (put == NULL)
		return ENOSYS;
	return (put) (devtab[devno].dev, c);
}

int dev_read(int devno, buf_t * b)
{
	dev_read_func_t read;

	disable;

	if (!valid_dev(devno) || !blk_dev(devno)) {
		enable;
		return EINVAL;
	}
	read = DEV_READ(devno);
	enable;
	if (read == NULL)
		return ENOSYS;
	return (read) (devtab[devno].dev, b);
}

int dev_shut(char *name)
{
	dev_shut_func_t shut;
	int i;

	if (name == NULL)
		return EINVAL;

	disable;

	for (i = 0; i < DEVICES && strcmp(devtab[i].name, name) != 0; i++) ;
	if (i == DEVICES) {
		enable;
		return EINVAL;
	}
	shut = devtab[i].ops.shut;
	enable;
	if (shut == NULL)
		return ENOSYS;
	return (shut) (devtab[i].dev);
}

int dev_uninst(char *name)
{
	int i;

	if (name == NULL)
		return EINVAL;

	disable;

	for (i = 0; i < DEVICES; i++)
		if (strcmp(devtab[i].name, name) == 0) {
			bzero(&(devtab[i]), sizeof(struct dev));

			enable;
			return 0;
		}
	enable;
	return ENXIO;
}

int dev_write(int devno, buf_t * b)
{
	dev_write_func_t write;

	disable;

	if (!valid_dev(devno) || !blk_dev(devno)) {
		enable;
		return EINVAL;
	}
	write = DEV_WRITE(devno);
	enable;
	if (write == NULL)
		return ENOSYS;
	return (write) (devtab[devno].dev, b);
}
