#include <fs/ramfs.h>
#include <stdio.h>

int
ramfs_lookup(file_t file, char *path)
{
	kprintf("ramfs_lookup: file->name [%s] path [%s]\n",
		file->path, path);
	return (-1);
}
