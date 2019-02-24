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

indent: clean
	cd cat; Indent
	cd cp; Indent
	cd fdisk; Indent
	cd format; Indent
	cd ls; Indent
	cd mkdir; Indent
	cd mount; Indent
	cd rm; Indent
	cd sh; Indent
	cd umount; Indent
	cd vi; Indent
