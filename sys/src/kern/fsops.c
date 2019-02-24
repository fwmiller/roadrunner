/* 
 *  Roadrunner/pk
 *    Copyright (C) 1989-2001  Cornfed Systems, Inc.
 *
 *  The Roadrunner/pk operating system is free software; you can
 *  redistribute and/or modify it under the terms of the GNU General
 *  Public License, version 2, as published by the Free Software
 *  Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this program; if not, write to the Free
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA 02111-1307 USA
 *
 *  More information about the Roadrunner/pk operating system of
 *  which this file is a part is available on the World-Wide Web
 *  at: http://www.cornfed.com.
 *
 */

#include <errno.h>
#include <fs.h>
#include <stdlib.h>
#include <string.h>

struct fsops fsopstab[FILE_SYSTEM_TYPES];
struct mutex fsopstabmutex;

int
fsops_init(fsops_t fsops)
{
    if (fsops->init != NULL)
	return fsops->init();
    return ENOSYS;
}

fsops_t
fsops_inst(fsops_t fsops)
{
    int fsopsno;

    mutex_lock(&fsopstabmutex);

    for (fsopsno = 0; fsopsno < FILE_SYSTEM_TYPES; fsopsno++)
	if (strcmp(fsopstab[fsopsno].name, fsops->name) == 0) {
	    mutex_unlock(&fsopstabmutex);
	    return NULL;
	}
    for (fsopsno = 0;
	 fsopsno < FILE_SYSTEM_TYPES
	 && fsopstab[fsopsno].name[0] != '\0'; fsopsno++);
    if (fsopsno == FILE_SYSTEM_TYPES) {
	mutex_unlock(&fsopstabmutex);
	return NULL;
    }
    strncpy(fsopstab[fsopsno].name, fsops->name, FS_NAME_LEN);
    fsopstab[fsopsno].init = fsops->init;
    fsopstab[fsopsno].shut = fsops->shut;
    fsopstab[fsopsno].mount = fsops->mount;
    fsopstab[fsopsno].unmount = fsops->unmount;
    fsopstab[fsopsno].open = fsops->open;
    fsopstab[fsopsno].close = fsops->close;
    fsopstab[fsopsno].ioctl = fsops->ioctl;
    fsopstab[fsopsno].read = fsops->read;
    fsopstab[fsopsno].write = fsops->write;
    fsopstab[fsopsno].attr = fsops->attr;
    fsopstab[fsopsno].readdir = fsops->readdir;
    fsopstab[fsopsno].unlink = fsops->unlink;

    mutex_unlock(&fsopstabmutex);
    return &(fsopstab[fsopsno]);
}

int
fsops_shut(fsops_t fsops)
{
    return ENOSYS;
}

int
fsops_uninst(fsops_t fsops)
{
    return ENOSYS;
}
