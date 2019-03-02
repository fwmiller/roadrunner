ROOTDIR = ../../..
include $(ROOTDIR)/Makefile.rules

CFLAGS += -D_KERNEL
CFLAGS += -D_DEBUG

OBJS = ramfile_attr.o ramfile_close.o ramfile_ioctl.o ramfile_open.o ramfile_read.o ramfile_readdir.o ramfile_unlink.o ramfile_write.o ramfs_init.o ramfs_mount.o ramfs_shut.o ramfs_unmount.o
TGT = $(LIBDIR)/libramfs.a

all: $(OBJS)
	$(AR) $(TGT) $(OBJS)
	$(RANLIB) $(TGT)

clean:
	$(RM) *~ $(OBJS) $(TGT)
