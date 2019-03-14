ROOTDIR = ../..
include $(ROOTDIR)/Makefile.rules

#CFLAGS += -D_DEBUG

OBJS = addch.o addstr.o clear.o clrtoeol.o getch.o getstr.o getyx.o inch.o move.o
TGT = $(ROOTDIR)/lib/libcurses.a

all: $(OBJS)
	$(AR) $(TGT) $(OBJS)
	$(RANLIB) $(TGT)

clean:
	$(RM) *~ $(OBJS) $(TGT)

indent: clean
	indent $(INDENT_RULES) *.c
	$(RM) *~
