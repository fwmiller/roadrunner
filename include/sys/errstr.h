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

char *errstr[] = {
  "success",
  "failure",			/* EFAIL */
  "argument list too long",	/* E2BIG */
  "permission denied",		/* EACCES */
  "resource temporarily unavailable",	/* EAGAIN */
  "bad file descriptor",	/* EBADF */
  "device busy",		/* EBUSY */
  "no child process",		/* ECHILD */
  "resource deadlock avoided",	/* EDEADLK */
  "numerical arg out of domain",	/* EDOM */
  "file exists",		/* EEXIST */
  "bad address",		/* EFAULT */
  "file too large",		/* EFBIG */
  "interrupted system call",	/* EINTR */
  "invalid argument",		/* EINVAL */
  "input/output error",		/* EIO */
  "is a directory",		/* EISDIR */
  "too many open files",	/* EMFILE */
  "too many links",		/* EMLINK */
  "file name too long",		/* ENAMETOOLONG */
  "too many open files in system",	/* ENFILE */
  "operation not supported by device",	/* ENODEV */
  "no such file or directory",	/* ENOENT */
  "exec format error",		/* ENOEXEC */
  "no locks available",		/* ENOLCK */
  "cannot allocate memory",	/* ENOMEM */
  "no space left on device",	/* ENOSPC */
  "function not implemented",	/* ENOSYS */
  "not a directory",		/* ENOTDIR */
  "directory not empty",	/* ENOTEMPTY */
  "inappropriate ioctl for device",	/* ENOTTY */
  "device not configured",	/* ENXIO */
  "operation not permitted",	/* EPERM */
  "broken pipe",		/* EPIPE */
  "result too large",		/* ERANGE */
  "read-only file system",	/* EROFS */
  "illegal seek",		/* ESPIPE */
  "no such process",		/* ESRCH */
  "cross-device link",		/* EXDEV */
  "protocol not supported",	/* EPROTONOSUPPORT */
  "connection refused",		/* ECONNREFUSED */
  "not connected",		/* ENOTCONN */
  "not a socket",		/* ENOTSOCK */
  "operation timed out",	/* ETIMEDOUT */
  "buffer in use",		/* EBUFINUSE */
  "missing file system type",	/* ENOFSTYPE */
  "file system already mounted",	/* EFSMOUNTED */
  "duplicate kmap entry",	/* EKMAPDUP */
  "duplicate page table entry",	/* EPTDUP */
  "corrupt page table entry",	/* EPTCORRUPT */
  "attempt to read past eof",	/* EFILEEOF */
  "bad cluster address",	/* EBADCLUST */
  "not a valid file system",	/* EBADFS */
  "device read error",		/* EDEVREAD */
  "device write error",		/* EDEVWRITE */
  "missing protocol control block",	/* ENOPCB */
};
