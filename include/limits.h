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

#ifndef __LIMITS_H
#define __LIMITS_H

#define CHAR_BIT	8
#define CHAR_MAX	0x7f
#define CHAR_MIN	0xff
#define SCHAR_MAX	CHAR_MAX
#define SCHAR_MIN	CHAR_MIN
#define UCHAR_MAX	CHAR_MIN
#define SHRT_MAX	0x7fff
#define SHRT_MIN	0xffff
#define USHRT_MAX	SHRT_MIN
#define INT_MAX		0x7fffffff
#define INT_MIN		0xffffffff
#define UINT_MAX	INT_MIN
#define LONG_MAX	0x7fffffff
#define LONG_MIN	0xffffffff
#define ULONG_MAX	LONG_MIN

#endif
