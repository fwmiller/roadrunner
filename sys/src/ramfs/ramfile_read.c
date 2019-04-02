#include <dev.h>
#include <errno.h>
#include <fs/ramfs.h>
#include <stdlib.h>
#include <sys/ioctl.h>

int ramfile_read(file_t file)
{
	int result;

	if (file->flags & F_EOF)
		return EINVAL;

	result = dev_ioctl(file->fs->devno, LOCK, NULL);
	if (result < 0)
		return result;

	result = dev_read(file->fs->devno, &(file->buf));

	dev_ioctl(file->fs->devno, UNLOCK, NULL);

	return result;
}
