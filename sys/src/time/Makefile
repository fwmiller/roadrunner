ROOTDIR = ../../..
include $(ROOTDIR)/Makefile.rules

CFLAGS += -D_KERNEL
CFLAGS += -D_DEBUG


OBJS = div.o time.o time2str.o time2tm.o timer.o tm2str.o tm2time.o tmr.o
TGT = $(LIBDIR)/libtime.a

all: $(OBJS)
	$(AR) $(TGT) $(OBJS)
	$(RANLIB) $(TGT)

clean:
	$(RM) *~ $(OBJS) $(TGT)
