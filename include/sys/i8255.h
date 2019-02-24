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

#ifndef __INTEL_8255_H
#define __INTEL_8255_H

/* I/O port addresses */
#define I8255_PORT_A		0x60
#define I8255_STATUS		0x64
#define I8255_CTRL		0x64

/* Status bits */
#define I8255_STATUS_OUTB	0x01
#define I8255_STATUS_INPB	0x02
#define I8255_STATUS_SYSF	0x04
#define I8255_STATUS_CD		0x08
#define I8255_STATUS_KEYL	0x10
#define I8255_STATUS_AUXB	0x20
#define I8255_STATUS_TIM	0x40
#define I8255_STATUS_PARE	0x80

/* Commands */
#define I8255_A20_HANDL		0xfd
#define I8255_REBOOT		0xfe

#endif
