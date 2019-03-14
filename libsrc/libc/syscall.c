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

#include <fs.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/utsname.h>

void _exit(int status)
{
	syscall(SYSCALL_EXIT, &status);
}

int attr(int fd, attrlist_t attr)
{
	return syscall(SYSCALL_ATTR, &fd);
}

int chdir(const char *path)
{
	return syscall(SYSCALL_CHDIR, &path);
}

int close(int fd)
{
	return syscall(SYSCALL_CLOSE, &fd);
}

pid_t exec(const char *path, int argc, char **argv)
{
	return syscall(SYSCALL_EXEC, &path);
}

void free(void *ptr)
{
	syscall(SYSCALL_FREE, &ptr);
}

char *getcwd(char *buf, size_t size)
{
	return (char *)syscall(SYSCALL_GETCWD, &buf);
}

int getfstab(fsrectab_t * fsrectab)
{
	return syscall(SYSCALL_GETFSTAB, &fsrectab);
}

pid_t getpid()
{
	return syscall(SYSCALL_GETPID, NULL);
}

int getstdpath(pid_t pid, int stdpath)
{
	return syscall(SYSCALL_GETSTDPATH, &pid);
}

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	return syscall(SYSCALL_GETTIMEOFDAY, &tv);
}

void halt()
{
	syscall(SYSCALL_HALT, NULL);
}

int ioctl(int fd, int cmd, void *args)
{
	return syscall(SYSCALL_IOCTL, &fd);
}

int load(char *path, char **prog, u_long * ldsize, char **start)
{
	return syscall(SYSCALL_LOAD, &path);
}

void *malloc(size_t size)
{
	return (void *)syscall(SYSCALL_MALLOC, &size);
}

int mkdir(const char *path, mode_t mode)
{
	return syscall(SYSCALL_MKDIR, &path);
}

int mount(const char *type, const char *path, const char *dev)
{
	return syscall(SYSCALL_MOUNT, &type);
}

int open(const char *path, int flags)
{
	return syscall(SYSCALL_OPEN, &path);
}

ssize_t read(int fd, void *buf, size_t nbytes)
{
	return syscall(SYSCALL_READ, &fd);
}

int readdir(int fd, char *entry)
{
	return syscall(SYSCALL_READDIR, &fd);
}

void reboot()
{
	syscall(SYSCALL_REBOOT, NULL);
}

void setstdpath(pid_t pid, int stdpath, int fd)
{
	syscall(SYSCALL_SETSTDPATH, &pid);
}

int uname(struct utsname *name)
{
	return syscall(SYSCALL_UNAME, &name);
}

int unlink(const char *path)
{
	return syscall(SYSCALL_UNLINK, &path);
}

int unmount(char *path)
{
	return syscall(SYSCALL_UNMOUNT, &path);
}

int wait(pid_t pid)
{
	return syscall(SYSCALL_WAIT, &pid);
}

ssize_t write(int fd, void *buf, size_t nbytes)
{
	return syscall(SYSCALL_WRITE, &fd);
}
