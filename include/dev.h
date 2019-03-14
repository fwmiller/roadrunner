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

#ifndef __DEV_H
#define __DEV_H

#include <sys/buf.h>

#define DEV_NAME_LEN		32

#if _KERNEL

#define DEV_TYPE_UNUSED		0
#define DEV_TYPE_CHAR		1
#define DEV_TYPE_BLK		2

#define DEV_GET(DEVNO) (devtab[DEVNO].ops.specific.char_ops.get)
#define DEV_PUT(DEVNO) (devtab[DEVNO].ops.specific.char_ops.put)
#define DEV_READ(DEVNO) (devtab[DEVNO].ops.specific.blk_ops.read)
#define DEV_WRITE(DEVNO) (devtab[DEVNO].ops.specific.blk_ops.write)

/* Device operations function prototypes */
typedef int (*dev_init_func_t) (void *dev);
typedef int (*dev_shut_func_t) (void *dev);
typedef int (*dev_ioctl_func_t) (void *dev, int cmd, void *args);
typedef int (*dev_get_func_t) (void *dev, int *c);
typedef int (*dev_put_func_t) (void *dev, int c);
typedef int (*dev_read_func_t) (void *dev, buf_t * b);
typedef int (*dev_write_func_t) (void *dev, buf_t * b);

/* Character device specific operations */
struct char_ops
{
  dev_get_func_t get;
  dev_put_func_t put;
};

/* Block device specific operations */
struct blk_ops
{
  dev_read_func_t read;
  dev_write_func_t write;
};

/* Device operations */
typedef struct dev_ops
{
  dev_init_func_t init;
  dev_shut_func_t shut;
  dev_ioctl_func_t ioctl;
  union
  {
    struct char_ops char_ops;
    struct blk_ops blk_ops;
  } specific;
} *dev_ops_t;

typedef struct dev
{
  char name[DEV_NAME_LEN];
  int type;
  struct dev_ops ops;
  void *dev;			/* Device specific parameters */
  int refcnt;
} *dev_t;

extern struct dev devtab[];

void dumpdevtab ();
void devtab_init ();
int dev_inst (char *name, int type, dev_ops_t ops, void *dev);
int dev_uninst (char *name);
int dev_init (char *name);
int dev_shut (char *name);

#endif /* _KERNEL */

int dev_open (char *name);
int dev_close (int devno);
int dev_ioctl (int devno, int cmd, void *args);
int dev_get (int devno, int *c);
int dev_put (int devno, int c);

#if _KERNEL

int dev_read (int devno, buf_t * b);
int dev_write (int devno, buf_t * b);

#else

int dev_read (int devno, char *buf, int *len);
int dev_write (int devno, char *buf, int *len);

#endif /* _KERNEL */

#endif
