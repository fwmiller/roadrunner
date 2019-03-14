#ifndef __HD_H
#define __HD_H

#include <sys/buf.h>

typedef int (*hd_ioctl_func_t) (void *part, int cmd, void *args);
typedef int (*hd_read_func_t) (void *part, buf_t * b);
typedef int (*hd_write_func_t) (void *part, buf_t * b);

struct hd
{
  hd_ioctl_func_t ioctl;
  hd_read_func_t read;
  hd_write_func_t write;
  void *part;
};

typedef struct hd *hd_t;

void hdtab_init ();
int hd_alloc (hd_t hd);
int hd_ioctl (void *dev, int cmd, void *args);
int hd_read (void *dev, buf_t * b);
int hd_write (void *dev, buf_t * b);

#endif
