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

#include <dev.h>
#include <errno.h>
#include <fcntl.h>
#include <fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys.h>
#include <sys/intr.h>
#include <sys/proc.h>
#include <sys/segment.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>

#define KERNEL_MODE							\
    disable;								\
    kern_data_segs();							\
    enable

#define USER_MODE							\
    disable;								\
    user_data_segs();							\
    enable

int
_syscall()
{
    register u_long eax asm("eax");
    register u_long ebx asm("ebx");
    volatile int syscallno = (int) eax;
    volatile void *params = (void *) ebx;

    KERNEL_MODE;

    switch (syscallno) {
    case SYSCALL_ATTR:
	{
	    int fileno;
	    file_t file;
	    int result;

	    fileno = *((int *) params);
	    if (fileno < 0 || fileno >= FILES) {
		USER_MODE;
		return EINVAL;
	    }
	    file = &(filetab[fileno]);
	    result = file_attr(file, *((attrlist_t *) (params + 4)));
	    USER_MODE;
	    return result;
	}

    case SYSCALL_CHDIR:
	{
	    int result = proc_chdir(*((char **) params));

	    USER_MODE;
	    return result;
	}

    case SYSCALL_CLOSE:
	{
	    int fileno;
	    file_t file;
	    int result;

	    fileno = *((int *) params);
	    if (fileno < 0 || fileno >= FILES) {
		USER_MODE;
		return EINVAL;
	    }
	    file = &(filetab[fileno]);
	    result = file_close(file);
	    USER_MODE;
	    return result;
	}

    case SYSCALL_EXEC:
	{
	    pid_t pid = proc_exec(*((char **) params),
				  *((int *) (params + 4)),
				  *((char ***) (params + 8)));

	    USER_MODE;
	    return (int) pid;
	}

    case SYSCALL_EXIT:
	proc_exit(*((int *) params));
	/* Not reached */

    case SYSCALL_FREE:
	free(*((void **) params));
	USER_MODE;
	return 0;		       /* Return value ignored */

    case SYSCALL_GETCWD:
	{
	    char *cwd = proc_getcwd(*((char **) params),
				    *((size_t *) (params + 4)));

	    USER_MODE;
	    return (int) cwd;
	}

    case SYSCALL_GETFSTAB:
	{
	    int result = fs_getfstab(*((fsrectab_t **) params));

	    USER_MODE;
	    return result;
	}

    case SYSCALL_GETPID:
	{
	    pid_t pid = proc_getpid();

	    USER_MODE;
	    return pid;
	}

    case SYSCALL_GETSTDPATH:
	{
	    int fd = proc_getstdpath(*((pid_t *) params),
				     *((int *) (params + 4)));

	    USER_MODE;
	    return fd;
	}

    case SYSCALL_GETTIMEOFDAY:
	{
	    timeval_t tv;

	    tv = *((timeval_t *) params);
	    if (tv == NULL) {
		USER_MODE;
		return EINVAL;
	    }
	    utime(&(tv->tv_sec), &(tv->tv_usec));
	    USER_MODE;
	    return 0;
	}

    case SYSCALL_HALT:
	halt();
	/* Not reached */

    case SYSCALL_IOCTL:
	{
	    int fileno;
	    file_t file;
	    int result;

	    fileno = *((int *) params);
	    if (fileno == STDIN)
		fileno = current->fd[PFD_STDIN];
	    else if (fileno == STDOUT)
		fileno = current->fd[PFD_STDOUT];
	    else if (fileno == STDERR)
		fileno = current->fd[PFD_STDERR];

	    if (fileno < 0 || fileno >= FILES) {
		USER_MODE;
		return EINVAL;
	    }
	    file = &(filetab[fileno]);
	    result = file_ioctl(file, *((int *) (params + 4)),
				*((void **) (params + 8)));
	    USER_MODE;
	    return result;
	}

    case SYSCALL_LOAD:
	{
	    int result = load(*((char **) params),
			      *((char ***) (params + 4)),
			      *((u_long **) (params + 8)),
			      *((char ***) (params + 12)));

	    USER_MODE;
	    return result;
	}

    case SYSCALL_MALLOC:
	{
	    void *ptr = malloc(*((size_t *) params));

	    USER_MODE;
	    return (int) ptr;
	}

    case SYSCALL_MKDIR:
	{
	    file_t file;
	    int result;

	    /* XXX Ignore mode parameter for the moment... */
	    result = file_open(*((char **) params),
			       O_MKDIR | O_CREAT | O_RDWR, &file);
	    if (result < 0) {
		USER_MODE;
		return result;
	    }
	    USER_MODE;
	    return file->slot;
	}

    case SYSCALL_MOUNT:
	{
	    fs_t fs;
	    int devno, i, result;

	    devno = dev_open(*((char **) (params + 8)));
	    if (devno < 0) {
		USER_MODE;
		return devno;
	    }
	    for (i = 0; i < FILE_SYSTEM_TYPES; i++)
		if (strcmp(fsopstab[i].name, "rrfs") == 0)
		    break;
	    if (i == FILE_SYSTEM_TYPES) {
		USER_MODE;
		return ENOFSTYPE;
	    }
	    result = fs_mount(&(fsopstab[i]), *((char **) (params + 4)),
			      devno, &fs);
	    USER_MODE;
	    return result;
	}

    case SYSCALL_OPEN:
	{
	    file_t file;
	    int result;

	    result = file_open(*((char **) params),
			       *((int *) (params + 4)), &file);
	    if (result < 0) {
		USER_MODE;
		return result;
	    }
	    USER_MODE;
	    return file->slot;
	}

    case SYSCALL_READ:
	{
	    int fileno;
	    file_t file;
	    int len, result;

	    fileno = *((int *) params);
	    if (fileno == STDIN)
		fileno = current->fd[PFD_STDIN];
	    else if (fileno == STDOUT)
		fileno = current->fd[PFD_STDOUT];
	    else if (fileno == STDERR)
		fileno = current->fd[PFD_STDERR];

	    if (fileno < 0 || fileno >= FILES) {
		USER_MODE;
		return EINVAL;
	    }
	    file = &(filetab[fileno]);
	    len = *((int *) (params + 8));
	    result = file_read(file, *((char **) (params + 4)), &len);
	    USER_MODE;
	    if (result < 0)
		return result;
	    return len;
	}

    case SYSCALL_READDIR:
	{
	    int fileno;
	    file_t file;
	    int result;

	    fileno = *((int *) params);
	    if (fileno < 0 || fileno >= FILES) {
		USER_MODE;
		return EINVAL;
	    }
	    file = &(filetab[fileno]);
	    result = file_readdir(file, *((char **) (params + 4)));
	    USER_MODE;
	    return result;
	}

    case SYSCALL_REBOOT:
	reboot();
	/* Not reached */

    case SYSCALL_SETSTDPATH:
	proc_setstdpath(*((pid_t *) params),
			*((int *) (params + 4)), *((int *) (params + 8)));
	USER_MODE;
	return 0;		       /* Return value ignored */

    case SYSCALL_UNAME:
	{
	    int result = uname(*((utsname_t *) params));

	    USER_MODE;
	    return result;
	}

    case SYSCALL_UNLINK:
	{
	    int result = file_unlink(*((char **) params));

	    USER_MODE;
	    return result;
	}

    case SYSCALL_UNMOUNT:
	{
	    fs_t fs;
	    int result;

	    fs = fs_lookup(*((char **) params));
	    if (fs == NULL) {
		USER_MODE;
		return EINVAL;
	    }
	    result = fs_unmount(fs);
	    USER_MODE;
	    return result;
	}

    case SYSCALL_WAIT:
	{
	    int result = proc_wait(*((pid_t *) params));

	    USER_MODE;
	    return result;
	}

    case SYSCALL_WRITE:
	{
	    int fileno;
	    file_t file;
	    int len, result;

	    fileno = *((int *) params);
	    if (fileno == STDIN)
		fileno = current->fd[PFD_STDIN];
	    else if (fileno == STDOUT)
		fileno = current->fd[PFD_STDOUT];
	    else if (fileno == STDERR)
		fileno = current->fd[PFD_STDERR];

	    if (fileno < 0 || fileno >= FILES) {
		USER_MODE;
		return EINVAL;
	    }
	    file = &(filetab[fileno]);
	    len = *((int *) (params + 8));
	    result = file_write(file, *((char **) (params + 4)), &len);
	    USER_MODE;
	    if (result < 0)
		return result;
	    return len;
	}

    default:
	USER_MODE;
	return ENOSYS;
    }
}
