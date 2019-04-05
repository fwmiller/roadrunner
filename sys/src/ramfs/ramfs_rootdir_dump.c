#include <fs/ramfs.h>
#include <stdio.h>
#include <string.h>

void ramfs_rootdir_dump()
{
	char s[32];
	int len, olen = 0, slen = 0;
	int i, j;

	/* Offset length */
	for (i = 0; i < rootdir_entries; i++) {
		memset(s, 0, 32);
		sprintf(s, "%u", rootdir[i].offset);
		len = strlen(s);

		if (len > olen)
			olen = len;
	}
	/* Size length */
	for (i = 0; i < rootdir_entries; i++) {
		memset(s, 0, 32);
		sprintf(s, "%u", rootdir[i].size);
		len = strlen(s);

		if (len > slen)
			slen = len;
	}
	/* Dump the in-memory root directory entries */
	for (i = 0; i < rootdir_entries; i++) {
		memset(s, 0, 32);
		sprintf(s, "%u", rootdir[i].offset);
		len = strlen(s);

		for (j = 0; j < olen - len + 1; j++)
			kprintf(" ");

		kprintf("%u", rootdir[i].offset);

		memset(s, 0, 32);
		sprintf(s, "%u", rootdir[i].size);
		len = strlen(s);

		for (j = 0; j < slen - len + 1; j++)
			kprintf(" ");

		kprintf("%u %s\n", rootdir[i].size, rootdir[i].name);
	}
}
