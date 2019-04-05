#include <fs/ramfs.h>
#include <string.h>

struct ramfile ramfiletab[RAM_FILES];
struct mutex ramfiletabmutex;

int ramfs_init()
{
	memset(ramfiletab, 0, RAM_FILES * sizeof(struct ramfile));
	mutex_clear(&ramfiletabmutex);

	return 0;
}
