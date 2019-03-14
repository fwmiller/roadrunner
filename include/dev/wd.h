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

#ifndef __WD_H
#define __WD_H

#if _KERNEL

#include <sys/buf.h>

void wd_bootparams ();
int wd_init ();
int wd_shut ();
int wd_ioctl (int cmd, void *args);
int wd_read (buf_t * b);
int wd_write (buf_t * b);

int wd0a_init ();
int wd0a_shut ();
int wd0a_ioctl (int cmd, void *args);
int wd0a_read (buf_t * b);
int wd0a_write (buf_t * b);

int wd0b_init ();
int wd0b_shut ();
int wd0b_ioctl (int cmd, void *args);
int wd0b_read (buf_t * b);
int wd0b_write (buf_t * b);

int wd0c_init ();
int wd0c_shut ();
int wd0c_ioctl (int cmd, void *args);
int wd0c_read (buf_t * b);
int wd0c_write (buf_t * b);

int wd0d_init ();
int wd0d_shut ();
int wd0d_ioctl (int cmd, void *args);
int wd0d_read (buf_t * b);
int wd0d_write (buf_t * b);

#endif /* _KERNEL */

#endif
