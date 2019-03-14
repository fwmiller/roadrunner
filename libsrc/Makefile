all:
	cd libc; make
	cd libcurses; make

clean:
	$(RM) *~
	cd libc; make clean
	cd libcurses; make clean

indent: clean
	cd libc; make indent
	cd libcurses; make indent
