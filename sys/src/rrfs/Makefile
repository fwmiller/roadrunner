ROOTDIR = ../../..
include $(ROOTDIR)/Makefile.rules

CFLAGS += -D_KERNEL
CFLAGS += -D_DEBUG

OBJS = rrfile_attr.o rrfile_close.o rrfile_create.o rrfile_ioctl.o rrfile_open.o rrfile_read.o rrfile_readdir.o rrfile_unlink.o rrfile_write.o rrfs.o rrfs_init.o rrfs_lookup.o rrfs_mount.o rrfs_shut.o rrfs_strategy.o rrfs_unmount.o rrfs_updatedir.o
TGT = $(LIBDIR)/librrfs.a

all: $(OBJS)
	$(AR) $(TGT) $(OBJS)
	$(RANLIB) $(TGT)

clean:
	$(RM) *~ $(OBJS) $(TGT)

indent:
	indent $(INDENT_RULES) *.c
	$(RM) *~

