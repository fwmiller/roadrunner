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

#ifndef __MOTOROLA_146818_H
#define __MOTOROLA_146818_H

/* I/O port addresses */
#define MC146818_ADDR		0x70
#define MC146818_DATA		0x71

/* Commands */
#define MC146818_SEC		0x00
#define MC146818_MIN		0x02
#define MC146818_HOUR		0x04
#define MC146818_WDAY		0x06
#define MC146818_MDAY		0x07
#define MC146818_MON		0x08
#define MC146818_YEAR		0x09
#define MC146818_HDTYPE		0x12

#endif
