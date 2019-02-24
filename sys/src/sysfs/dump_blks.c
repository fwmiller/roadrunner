#include <stdio.h>
#include <sys/buf.h>
#include <sys/intr.h>

void
dump_blks(char *s)
{
    disable;
    sprintf(s, "%d\n", blkpool.nblks);
    enable;
}
