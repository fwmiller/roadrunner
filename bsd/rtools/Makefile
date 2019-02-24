CC = gcc
RM = rm -f

CFLAGS = -c -Wall
CFLAGS += -D_DEBUG

LIBOBJS = bufdump.o getdir.o getname.o printdirentry.o printmbr.o rrfs.o rrfs_find.o rrfs_mkdirentry.o
TGTOBJS = rcopy.o rdir.o rformat.o rmkdir.o rtype.o
TGTS = ../bin/rcopy ../bin/rdir ../bin/rformat ../bin/rmkdir ../bin/rtype

all: $(LIBOBJS) $(TGTOBJS)
	$(CC) -o ../bin/rcopy $(LIBOBJS) rcopy.o
	$(CC) -o ../bin/rdir $(LIBOBJS) rdir.o
	$(CC) -o ../bin/rformat $(LIBOBJS) rformat.o
	$(CC) -o ../bin/rmkdir $(LIBOBJS) rmkdir.o
	$(CC) -o ../bin/rtype $(LIBOBJS) rtype.o

clean:
	$(RM) *~ $(LIBOBJS) $(TGTOBJS) $(TGTS)

indent: clean
	Indent

.c.o: $<
	$(CC) $(CFLAGS) -I. -o $@ $<

