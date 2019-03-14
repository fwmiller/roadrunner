ROOTDIR = ../..
include $(ROOTDIR)/Makefile.rules
LIB = -nostdlib -L../../lib

#CFLAGS += -D_DEBUG
LDFLAGS = -u __startup
LDFLAGS += -m elf_i386

OBJS = command.o cursor.o delete.o disp.o file.o help.o insert.o line.o vi.o
LIBS = -lcurses -lc
TGT = ../../bin/vi

all: $(OBJS)
	$(LD) $(LDFLAGS) -r $(LIB) -o $(TGT) $(OBJS) $(LIBS)

clean:
	$(RM) *~ $(OBJS) $(TGT)

indent: clean
	indent $(INDENT_RULES) *.c
	$(RM) *~

