#include <errno.h>
#include <fs.h>
#include <fs/ramfs.h>
#include <stdio.h>
#include <string.h>

int ramfs_lookup(file_t file, char *path)
{
#if 0
	int i;
	int k = 0;

	kprintf("ramfs_lookup: file->path [%s] path [%s]\n", file->path, path);

	if (path[0] == '/')
		k = 1;

	for (i = 0; i < ramfs_entries; i++)
		if (strcmp(path + k, ramfiletab[i].name) == 0) {
			kprintf("ramfs_lookup: found\n");
			file->filesize = ramfiletab[i].size;
			return 0;
		}
#endif
	return ENOENT;
}
