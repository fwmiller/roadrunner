#include <errno.h>
#include <fs.h>
#include <fs/ramfs.h>
#include <stdio.h>
#include <string.h>

int
ramfs_lookup(file_t file, char *path)
{
	int filepos = 0;
	int i;

	kprintf("ramfs_lookup: file->path [%s] path [%s]\n",
		file->path, path);

	if (path[0] == '/')
		filepos = 1;

	for (i = 0; i < ramfs_entries; i++) {
		if (strcmp(path + filepos, dir[i].name) == 0) {
			kprintf("ramfs_lookup: found\n");
			return 0;
		}
	}
	return ENOENT;
}
