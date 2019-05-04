#include <errno.h>
#include <fs.h>
#include <fs/ramfs.h>
#include <stdio.h>
#include <string.h>

int ramfs_lookup(file_t file, char *path)
{
	int i;
	int k = 0;

	kprintf("ramfs_lookup: file->path [%s] path [%s]\n",
		file->path, path);

	if (path[0] == '/')
		k = 1;

	for (i = 0; i < rootdir_entries; i++) {
		if (strcmp(path + k, rootdir[i].name) == 0) {
			ramfile_t ramfile = (ramfile_t) file->data;

			ramfile->direntry = &(rootdir[i]);
			file->filesize = ramfile->direntry->size;
			ramfile->blkno =
				ramfile->direntry->offset + ramfiles_blkno;
			return 0;
		}
	}
	return ENOENT;
}
