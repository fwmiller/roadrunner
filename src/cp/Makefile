ROOTDIR = ../..
include $(ROOTDIR)/Makefile.rules
LIB = -nostdlib -L../../lib

#CFLAGS += -D_DEBUG
LDFLAGS = -u __startup
LDFLAGS += -m elf_i386

OBJS = cp.o
LIBS = -lc
TGT = ../../bin/cp

all: $(OBJS)
	$(LD) $(LDFLAGS) -r $(LIB) -o $(TGT) $(OBJS) $(LIBS)

clean:
	$(RM) *~ $(OBJS) $(TGT)

indent: clean
	indent $(INDENT_RULES) *.c
	$(RM) *~

