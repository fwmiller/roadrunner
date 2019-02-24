ROOTDIR = ../..
include $(ROOTDIR)/Makefile.rules
LIB = -nostdlib -L../../lib

CFLAGS += -D_DEBUG
LDFLAGS = -u __startup
LDFLAGS += -m elf_i386

OBJS = format.o
LIBS = -lc
TGT = ../../bin/format

all: $(OBJS)
	$(LD) $(LDFLAGS) -r $(LIB) -o $(TGT) $(OBJS) $(LIBS)

clean:
	$(RM) *~ $(OBJS) $(TGT)

indent: clean
	Indent
