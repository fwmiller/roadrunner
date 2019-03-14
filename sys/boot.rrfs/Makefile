ROOTDIR = ../..
include $(ROOTDIR)/Makefile.rules

NASM = nasm -f bin

CFLAGS += -D_KERNEL
CFLAGS += -Os
#CFLAGS += -D_DEBUG

# boot2.o must be first
BOOT2OBJS = boot2.o asm.o ata.o cons.o fd.o gatea20.o intr.o main.o printf.o string.o sys.o
OBJS = $(BOOT2OBJS)

all: $(OBJS)
	$(NASM) boot1.asm
	ld -m elf_i386 -N -Ttext 2000 -o boot2.sym $(BOOT2OBJS)
	objcopy -R .comment -R .note -S -O binary boot2.sym boot2
	cat boot1 boot2 > boot

clean:
	$(RM) *~ $(OBJS) boot1 boot1.sym boot2 boot2.sym boot *.obj *.lst *.bin

indent: clean
	indent $(INDENT_RULES) *.c
	$(RM) *~

