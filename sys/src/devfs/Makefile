ROOTDIR = ../../..
include $(ROOTDIR)/Makefile.rules

CFLAGS += -D_KERNEL
CFLAGS += -D_DEBUG

OBJS = devfile_attr.o devfile_close.o devfile_ioctl.o devfile_open.o devfile_read.o devfile_readdir.o devfile_unlink.o devfile_write.o devfs_init.o devfs_mount.o devfs_shut.o devfs_unmount.o
TGT = $(LIBDIR)/libdevfs.a

all: $(OBJS)
	$(AR) $(TGT) $(OBJS)
	$(RANLIB) $(TGT)

clean:
	$(RM) *~ $(OBJS) $(TGT)

indent:
	indent $(INDENT_RULES) *.c
	$(RM) *~

