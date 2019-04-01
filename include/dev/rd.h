#ifndef __RAMDISK_H
#define __RAMDISK_H

#include <sys/buf.h>

int rd_init (void *dev);
int rd_shut (void *dev);
int rd_ioctl (void *dev, int cmd, void *args);
int rd_read (void *dev, buf_t * b);
int rd_write (void *dev, buf_t * b);

#endif
