/* 
 *  Roadrunner/pk
 *    Copyright (C) 1989-2002  Cornfed Systems, Inc.
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

#ifndef __CONFIG_H
#define __CONFIG_H

#define COPYRIGHT "Copyright (C) 1989-2002 Cornfed Systems Inc"

#define SYSNAME			"Roadrunner"
#define NODENAME		""
#define RELEASE			"16"
#define VERSION			"0.8"
#define MACHINE			"i386"

#define SECTOR_SIZE		512
#define PAGE_SIZE		4096
#define LINE_LENGTH		PAGE_SIZE
#define PATH_LENGTH		PAGE_SIZE
#define FS_NAME_LEN		32
#define RRFS_BOOT_SECTORS	16

/* Console parameters */
#define COLS			80
#define LINES			25

/* Keyboard keystrokes */
#define ESC			0x1b
#define NUM_LCK			0x80
#define SCR_LCK			0x81
#define HOME			0x82
#define UP			0x83
#define PG_UP			0x84
#define LEFT			0x85
#define RIGHT			0x86
#define END			0x87
#define DOWN			0x88
#define PG_DOWN			0x89
#define INS			0x8a
#define DEL			0x8b

#if _KERNEL

#define CLOCK			50     /* Hz */
#define REGIONS			2048
#define VM_KMAP_ENTRIES		32
#define INTRS			64
#define ISRS			INTRS
#define PRIORITIES		32
#define PAGE_TABLES		16
#define PROCS			PAGE_TABLES
#define STACK_SIZE		(16 * PAGE_SIZE)
#define EVENTS			16
#define DEVICES			32
#define BLKS			64
#define BUFS			64
#define BUFQS			16
#define FILE_SYSTEM_TYPES	4
#define FILE_SYSTEMS		8
#define FILES			32

#endif				/* _KERNEL */

#endif
