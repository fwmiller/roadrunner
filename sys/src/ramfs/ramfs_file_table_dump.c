#include <fs/ramfs.h>
#include <stdio.h>
#include <string.h>

void
ramfs_file_table_dump(int entries)
{
	char size[32];
	int sizelen;
	int i, j;
	int len = 0;

	for (i = 0; i < entries; i++) {
		memset(size, 0, 32);
		sprintf(size, "%u", dir[i].size);
		sizelen = strlen(size);

		if (sizelen > len)
			len = sizelen;
	}
	/* Dump the file table entries */
	for (i = 0; i < entries; i++) {
		memset(size, 0, 32);
		sprintf(size, "%u", dir[i].size);
		sizelen = strlen(size);

		for (j = 0; j < len - sizelen + 1; j++)
			kprintf(" ");

		kprintf("%u %s\n", dir[i].size, dir[i].name);
	}
}
