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

#ifndef __STAT_H
#define __STAT_H

#include <sys/types.h>

#define S_IRUSR		0x01	       /* R for owner */
#define S_IWUSR		0x02	       /* W for owner */
#define S_IXUSR		0x04	       /* X for owner */

/* RWX mask for owner */
#define S_IRWXU		(S_IRUSR | S_IWUSR | S_IXUSR)

#define S_IFDIR		0x08	       /* Directory */

/* POSIX */
#define S_IREAD		S_IRUSR
#define S_IWRITE	S_IWUSR
#define S_IEXEC		S_IXUSR

#endif
