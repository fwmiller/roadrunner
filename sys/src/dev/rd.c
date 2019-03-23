#include <dev/rd.h>

int rd_init(void *dev)
{
	
	return 0;
}

int rd_shut(void *dev)
{
	return (-1);
}

int rd_ioctl(void *dev, int cmd, void *args)
{
	return (-1);
}

int rd_read(void *dev, buf_t * b)
{
	return (-1);
}

int rd_write(void *dev, buf_t * b)
{
	return (-1);
}

