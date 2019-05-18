ROOTDIR = ../..
include $(ROOTDIR)/Makefile.rules

CFLAGS += -D_DEBUG

OBJS = _syscall.o argv.o atoi.o bcopy.o bufdump.o bzero.o clearerr.o crt0.o fclose.o feof.o ferror.o fflush.o fgetc.o fgetpos.o fgets.o fopen.o fprintf.o fputc.o fputs.o fread.o freopen.o fscanf.o fseek.o fsetpos.o ftell.o fwrite.o getc.o getchar.o gets.o lseek.o memcmp.o memcpy.o memmove.o nextarg.o part.o path.o perror.o print.o printf.o putc.o putchar.o puts.o realloc.o remove.o rename.o rewind.o scanf.o sprintf.o sscanf.o strcat.o strchr.o strcmp.o strcpy.o strcspn.o strerror.o strlen.o strncat.o strncmp.o strncpy.o strrchr.o strspn.o strstr.o sys.o syscall.o time2tm.o tm2str.o tmpfile.o uint2str.o unget.o vfprintf.o vprintf.o vsprintf.o
TGT = $(ROOTDIR)/lib/libc.a

all: $(OBJS)
	$(AR) $(TGT) $(OBJS)
	$(RANLIB) $(TGT)

clean:
	$(RM) *~ $(OBJS) $(TGT)

indent: clean
	indent $(INDENT_RULES) *.c
	$(RM) *~
