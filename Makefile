#BOOT = sys/boot/boot
KERNEL = sys/src/kern/os
IMAGE = ./image

all: elements
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

elements:
#	cd bsd; make
	cd libsrc; make
	cd src; make
#	cd sys/boot; make
	cd sys/src/kern; make

doc:
	cd doc; make

clean:
	rm -f *~ $(IMAGE)
#	cd bsd; make clean
	cd doc; make clean
	cd libsrc; make clean
	cd src; make clean
#	cd sys/boot; make clean
	cd sys/src/kern; make clean

indent:
	rm -f *~ $(IMAGE)
#	cd bsd; make indent
	cd doc; make clean
	cd libsrc; make indent
	cd src; make indent
#	cd sys/boot; make indent
	cd include; Indent
	cd include/bus; Indent
	cd include/dev; Indent
	cd include/fs; Indent
	cd include/sys; Indent
	cd sys/src/kern; make indent
