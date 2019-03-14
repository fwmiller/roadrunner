#include <fs/ramfs.h>
#include <stdlib.h>
#include <string.h>

void
ramfile_clear(ramfile_t rf)
{
	if (rf == NULL)
		return;

	memset(rf, 0, sizeof(struct ramfile));
}
