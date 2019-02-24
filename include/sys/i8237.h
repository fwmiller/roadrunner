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

#ifndef __INTEL_8237_H
#define __INTEL_8237_H

/* I/O port addresses */
#define I8237_DMA1_CHAN		0x0a
#define I8237_DMA1_MODE		0x0b
#define I8237_DMA1_RESET	0x0c

#define I8237_DMA1_CHAN2_ADDR	0x04
#define I8237_DMA1_CHAN2_COUNT	0x05
#define I8237_DMA1_CHAN2_PAGE	0x81

/* Commands */
#define I8237_DMA1_CHAN2_READ	0x46
#define I8237_DMA1_CHAN2_WRITE	0x4a

#endif
