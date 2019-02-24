ROOTDIR = ../..
include $(ROOTDIR)/Makefile.rules
LIB = -nostdlib -L../../lib

#CFLAGS += -D_DEBUG
LDFLAGS = -u __startup
LDFLAGS += -m elf_i386

OBJS = rm.o
LIBS = -lc
TGT = ../../bin/rm

all: $(OBJS)
	$(LD) $(LDFLAGS) -r $(LIB) -o $(TGT) $(OBJS) $(LIBS)

clean:
	$(RM) *~ $(OBJS) $(TGT)

indent: clean
	Indent
