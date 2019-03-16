#include <errno.h>
#include <fcntl.h>
#include <fs/ramfs.h>
#include <stdio.h>

int ramfile_open(file_t file)
{
	ramfile_t ramfile;
	int ramfileno, result;

	mutex_lock(&ramfiletabmutex);

	for (ramfileno = 0;
	     ramfileno < RAM_FILES &&
	     ramfiletab[ramfileno].flags & RAMF_INUSE;
	     ramfileno++);
	if (ramfileno == RAM_FILES) {
#if _DEBUG
		kprintf("ramfile_open: ramfiletab full\n");
#endif
		mutex_unlock(&ramfiletabmutex);
		return EAGAIN;
	}
	ramfile = (ramfile_t) &(ramfiletab[ramfileno]);
	ramfile->flags |= RAMF_INUSE;
	mutex_unlock(&ramfiletabmutex);

	file->data = ramfile;
	file->bufsize = RAMFILE_BUFSIZE;

	if (file->flags & O_RDONLY) {
		result = ramfs_lookup(file, file->path);
		if (result < 0)
			goto openerror;
	}
	return EINVAL;

openerror:
	mutex_lock(&ramfiletabmutex);
	ramfile_clear(ramfile);
	mutex_unlock(&ramfiletabmutex);
	return result;
}
