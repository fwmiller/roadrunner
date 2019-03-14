RAMFS = ../bin/ramfs
RAMFS_C = ../sys/src/ramfs/ramfs_content.c

all:
	cd cat; make
	cd cp; make
	cd fdisk; make
	cd format; make
	cd ls; make
	cd mkdir; make
	cd mount; make
	cd rm; make
	cd sh; make
	cd umount; make
	cd vi; make
	./mkramfs.py
	xxd --include $(RAMFS) > $(RAMFS_C)

clean:
	$(RM) *~
	cd cat; make clean
	cd cp; make clean
	cd fdisk; make clean
	cd format; make clean
	cd ls; make clean
	cd mkdir; make clean
	cd mount; make clean
	cd rm; make clean
	cd sh; make clean
	cd umount; make clean
	cd vi; make clean
	rm -f $(RAMFS) $(RAMFS_C)

indent: clean
	cd cat; make indent
	cd cp; make indent
	cd fdisk; make indent
	cd format; make indent
	cd ls; make indent
	cd mkdir; make indent
	cd mount; make indent
	cd rm; make indent
	cd sh; make indent
	cd umount; make indent
	cd vi; make indent
