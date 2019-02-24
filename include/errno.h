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

#ifndef __ERRNO_H
#define __ERRNO_H

#define EFAIL		(-1)	       /* Generic failure */

/* POSIX.1 error codes */
#define E2BIG		(-2)	       /* Argument list too long */
#define EACCES		(-3)	       /* Permission denied */
#define EAGAIN		(-4)	       /* Resource temporarily unavail */
#define EBADF		(-5)	       /* Bad file descriptor */
#define EBUSY		(-6)	       /* Device busy */
#define ECHILD		(-7)	       /* No child processes */
#define EDEADLK		(-8)	       /* Resource deadlock avoided */
#define EDOM		(-9)	       /* Numerical arg out of domain */
#define EEXIST		(-10)	       /* File exists */
#define EFAULT		(-11)	       /* Bad address */
#define EFBIG		(-12)	       /* File too large */
#define EINTR		(-13)	       /* Interrupted system call */
#define EINVAL		(-14)	       /* Invalid argument */
#define EIO		(-15)	       /* Input/output error */
#define EISDIR		(-16)	       /* Is a directory */
#define EMFILE		(-17)	       /* Too many open files */
#define EMLINK		(-18)	       /* Too many links */
#define ENAMETOOLONG	(-19)	       /* File name too long */
#define ENFILE		(-20)	       /* Too many open files in system */
#define ENODEV		(-21)	       /* Operation not supported by device */
#define ENOENT		(-22)	       /* No such file or directory */
#define ENOEXEC		(-23)	       /* Exec format error */
#define ENOLCK		(-24)	       /* No locks avail */
#define ENOMEM		(-25)	       /* Cannot allocate memory */
#define ENOSPC		(-26)	       /* No space left on device */
#define ENOSYS		(-27)	       /* Function not implemented */
#define ENOTDIR		(-28)	       /* Not a directory */
#define ENOTEMPTY	(-29)	       /* Directory not empty */
#define ENOTTY		(-30)	       /* Inappropriate ioctl for device */
#define ENXIO		(-31)	       /* Device not configured */
#define EPERM		(-32)	       /* Operation not permitted */
#define EPIPE		(-33)	       /* Broken pipe */
#define ERANGE		(-34)	       /* Result too large */
#define EROFS		(-35)	       /* Read-only file system */
#define ESPIPE		(-36)	       /* Illegal seek */
#define ESRCH		(-37)	       /* No such process */
#define EXDEV		(-38)	       /* Cross-device link */

/* Socket specific error codes */
#define EPROTONOSUPPORT	(-39)	       /* Protocol not supported */
#define ECONNREFUSED	(-40)	       /* Connection refused */
#define ENOTCONN	(-41)	       /* Not connected */
#define ENOTSOCK	(-42)	       /* Not a socket */

/* Roadrunner/pk specific error codes */
#define ETIMEDOUT	(-43)	       /* Operation timed out */
#define EBUFINUSE	(-44)	       /* Buffer in use */
#define ENOFSTYPE	(-45)	       /* Missing file system type */
#define EFSMOUNTED	(-46)	       /* File system already mounted */
#define EKMAPDUP	(-47)	       /* Duplicate kmap entry */
#define EPTDUP		(-48)	       /* Duplicate page table entry */
#define EPTCORRUPT	(-49)	       /* Corrupt page table entry */
#define EFILEEOF	(-50)	       /* Attempt to read past eof */
#define EBADCLUST	(-51)	       /* Bad cluster address */
#define EBADFS		(-52)	       /* Not a valid file system */
#define EDEVREAD	(-53)	       /* Device read error */
#define EDEVWRITE	(-54)	       /* Device write error */
#define ENOPCB		(-55)	       /* Missing protocol control block */

/*  Must be last */
#define EMIN		(-56)

extern int errno;

#endif
