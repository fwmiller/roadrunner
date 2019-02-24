ROOTDIR = ../../..
include $(ROOTDIR)/Makefile.rules

CFLAGS += -D_KERNEL
CFLAGS += -D_DEBUG

OBJS = dump_blks.o dump_bufs.o dump_filetab.o dump_fstab.o dump_proctab.o dump_regiontab.o dump_vm.o sysfile_attr.o sysfile_close.o sysfile_ioctl.o sysfile_open.o sysfile_read.o sysfile_readdir.o sysfile_unlink.o sysfile_write.o sysfs_init.o sysfs_mount.o sysfs_shut.o sysfs_unmount.o
TGT = $(LIBDIR)/libsysfs.a

all: $(OBJS)
	$(AR) $(TGT) $(OBJS)
	$(RANLIB) $(TGT)

clean:
	$(RM) *~ $(OBJS) $(TGT)
