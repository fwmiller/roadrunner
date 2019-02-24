ROOTDIR = ../../..
include $(ROOTDIR)/Makefile.rules

CFLAGS += -D_KERNEL
CFLAGS += -D_DEBUG
CFLAGS += -D_DEBUG_PCI

OBJS = ata.o cons.o fd.o hd.o kbd.o part.o pci.o uart.o
TGT = $(LIBDIR)/libdev.a

all: $(OBJS)
	$(AR) $(TGT) $(OBJS)
	$(RANLIB) $(TGT)

clean:
	$(RM) *~ $(OBJS) $(TGT)

indent: clean
	Indent
