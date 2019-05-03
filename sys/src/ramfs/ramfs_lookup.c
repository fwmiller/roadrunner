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

		kprintf("ramfs_lookup: path [%s] rootdir[i].name [%s]\n",
			path + k, rootdir[i].name);

		if (strcmp(path + k, rootdir[i].name) == 0) {
			ramfile_t ramfile = (ramfile_t) file->data;

			file->filesize = rootdir[i].size;
			ramfile->direntry = &(rootdir[i]);
#if _DEBUG
			kprintf("ramfs_lookup: found offset %d\n",
				ramfile->direntry->offset);
#endif
			ramfile->blkno = rootdir[i].offset;
#if _DEBUG
			kprintf("ramfs_lookup: blkno %d\n",
				ramfile->blkno);
#endif
			return 0;
		}
	}
	return ENOENT;
}
