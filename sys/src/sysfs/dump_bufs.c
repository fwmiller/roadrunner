#include <stdio.h>
#include <sys/buf.h>
#include <sys/intr.h>

void
dump_bufs(char *s)
{
    disable;
    sprintf(s, "%d\n", bufpool.nbufs);
    enable;
}
