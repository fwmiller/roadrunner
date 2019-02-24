ROOTDIR = ../../..
include $(ROOTDIR)/Makefile.rules

CFLAGS += -D_KERNEL
CFLAGS += -D_DEBUG

OBJS = dump.o load.o reloc.o sect.o
TGT = $(LIBDIR)/libload.a

all: $(OBJS)
	$(AR) $(TGT) $(OBJS)
	$(RANLIB) $(TGT)

clean:
	$(RM) *~ $(OBJS) $(TGT)
