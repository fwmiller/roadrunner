#include <fs/ramfs.h>
#include <stdio.h>
#include <string.h>

void ramfs_file_table_dump(int entries)
{
	char s[32];
	int len, olen = 0, slen = 0;
	int i, j;

	/* Offset length */
	for (i = 0; i < entries; i++) {
		memset(s, 0, 32);
		sprintf(s, "%u", ramfiletab[i].offset);
		len = strlen(s);

		if (len > olen)
			olen = len;
	}
	/* Size length */
	for (i = 0; i < entries; i++) {
		memset(s, 0, 32);
		sprintf(s, "%u", ramfiletab[i].size);
		len = strlen(s);

		if (len > slen)
			slen = len;
	}
	/* Dump the file table entries */
	for (i = 0; i < entries; i++) {
		memset(s, 0, 32);
		sprintf(s, "%u", ramfiletab[i].offset);
		len = strlen(s);

		for (j = 0; j < olen - len + 1; j++)
			kprintf(" ");

		kprintf("%u", ramfiletab[i].offset);

		memset(s, 0, 32);
		sprintf(s, "%u", ramfiletab[i].size);
		len = strlen(s);

		for (j = 0; j < slen - len + 1; j++)
			kprintf(" ");

		kprintf("%u %s\n", ramfiletab[i].size, ramfiletab[i].name);
	}
}
