ROOTDIR = ../../..
include $(ROOTDIR)/Makefile.rules

CFLAGS += -D_KERNEL
CFLAGS += -D_DEBUG
LDFLAGS = -Bstatic -Ttext 100000 
LDFLAGS += -m elf_i386

ISODIR = $(ROOTDIR)/sys/boot/isofiles
ISOIMG = $(ROOTDIR)/sys/boot/img.iso

QEMU = qemu-system-i386
QEMUFLAGS = -m 256M -nographic -no-reboot -drive format=raw,file=$(ISOIMG)

#
# init.o must be first
#
OBJS = mb2.o init.o asm.o blk.o boot.o buf.o bufq.o cpuid.o dev.o event.o file.o fs.o fsops.o intr.o mem.o mutex.o nextlex.o os.o path.o print.o proc.o queue.o region.o segment.o string.o sys.o syscall.o tsc.o uname.o vm.o
LIBS = -lload -lramfs -lrrfs -ldevfs -ldev -lsysfs -ltime

TGT = os

all: kern
	cd $(ROOTDIR)/sys/src/dev; make
	cd $(ROOTDIR)/sys/src/devfs; make
	cd $(ROOTDIR)/sys/src/load; make
	cd $(ROOTDIR)/sys/src/ramfs; make
	cd $(ROOTDIR)/sys/src/rrfs; make
	cd $(ROOTDIR)/sys/src/sysfs; make
	cd $(ROOTDIR)/sys/src/time; make
	make build
	cp os $(ISODIR)/boot/kern.elf
	grub-mkrescue -o $(ISOIMG) $(ISODIR)

run: all
	$(QEMU) $(QEMUFLAGS)

kern: $(OBJS)

build:
	$(LD) $(LDFLAGS) -nostdlib -L$(LIBDIR) -o $(TGT) $(OBJS) $(LIBS)
	$(SIZE) $(TGT)

clean: kernclean
	cd $(ROOTDIR)/sys/src/dev; make clean
	cd $(ROOTDIR)/sys/src/devfs; make clean
	cd $(ROOTDIR)/sys/src/load; make clean
	cd $(ROOTDIR)/sys/src/ramfs; make clean
	cd $(ROOTDIR)/sys/src/rrfs; make clean
	cd $(ROOTDIR)/sys/src/sysfs; make clean
	cd $(ROOTDIR)/sys/src/time; make clean
	rm -f $(ISODIR)/boot/kern.elf
	rm -f $(ISOIMG)

kernclean:
	$(RM) *~ $(OBJS) $(TGT).sym $(TGT)

indent: clean
	Indent
	cd $(ROOTDIR)/sys/src/dev; Indent
	cd $(ROOTDIR)/sys/src/devfs; Indent
	cd $(ROOTDIR)/sys/src/load; Indent
	cd $(ROOTDIR)/sys/src/ramfs; Indent
	cd $(ROOTDIR)/sys/src/rrfs; Indent
	cd $(ROOTDIR)/sys/src/sysfs; Indent
	cd $(ROOTDIR)/sys/src/time; Indent
