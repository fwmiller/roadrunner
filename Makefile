#BOOT = sys/boot/boot
KERNEL = sys/src/kern/os
#IMAGE = ./image

#all: elements
#	rm -f $(IMAGE)
#	dd if=/dev/zero of=$(IMAGE) ibs=512 count=2880
#	bsd/bin/rformat -d $(IMAGE) -b $(BOOT)
#	bsd/bin/rcopy -d $(IMAGE) $(KERNEL) /kernel
#	bsd/bin/rcopy -d $(IMAGE) $(BOOT) /boot
#	bsd/bin/rmkdir -d $(IMAGE) /bin
#	bsd/bin/rcopy -d $(IMAGE) bin/* /bin

#floppy: elements
#	bsd/bin/rformat -b $(BOOT)
#	bsd/bin/rcopy $(KERNEL) /kernel
#	bsd/bin/rcopy $(BOOT) /boot
#	bsd/bin/rmkdir /bin
#	bsd/bin/rcopy bin/* /bin

all:
#	cd bsd; make
	cd libsrc; make
	cd src; make
#	cd sys/boot; make
	cd sys/src/kern; make

run: all
	cd sys/src/kern; make run

doc:
	cd doc; make

clean:
#	rm -f *~ $(IMAGE)
#	cd bsd; make clean
	cd doc; make clean
	cd libsrc; make clean
	cd src; make clean
#	cd sys/boot; make clean
	cd sys/src/kern; make clean

indent:
#	rm -f *~ $(IMAGE)
	cd bsd; make indent
	cd libsrc; make indent
	cd src; make indent
	cd include; indent $(INDENT_RULES) *.h; $(RM) *~
	cd include/bus; indent $(INDENT_RULES) *.h; $(RM) *~
	cd include/dev; indent $(INDENT_RULES) *.h; $(RM) *~
	cd include/fs; indent $(INDENT_RULES) *.h; $(RM) *~
	cd include/sys; indent $(INDENT_RULES) *.h; $(RM) *~
	cd sys/src/kern; make indent

wc:
	wc -l include/*.h include/bus/*.h include/dev/*.h include/fs/*.h include/sys/*.h libsrc/libc/*.c libsrc/libcurses/*.c src/cat/*.c src/cp/*.c src/fdisk/*.c src/format/*.c src/ls/*.c src/mkdir/*.c src/mount/*.c src/rm/*.c src/sh/*.c src/umount/*.c src/vi/*.c sys/src/dev/*.c sys/src/devfs/*.c sys/src/kern/*.c sys/src/load/*.c sys/src/ramfs/*.c sys/src/rrfs/*.c sys/src/sysfs/*.c sys/src/time/*.c
