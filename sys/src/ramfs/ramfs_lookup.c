#include <errno.h>
#include <fs.h>
#include <fs/ramfs.h>
#include <stdio.h>
#include <string.h>

int
ramfs_lookup(file_t file, char *path)
{
	int pos = 0;
	int i;

	kprintf("ramfs_lookup: file->name [%s] path [%s]\n",
		file->path, path);

	if (path[0] == '/')
		pos = 1;

	for (i = 0; i < ramfs_entries; i++) {
		if (strcmp(path + pos, dir[i].name) == 0) {
			kprintf("ramfs_lookup: found\n");
			return 0;
		}
	}
	return ENOENT;
}
