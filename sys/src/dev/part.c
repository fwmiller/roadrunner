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

#include <stdio.h>
#include <string.h>
#include <sys.h>
#include <sys/part.h>

void read_parttab(u_char * mbr, part_t parttab)
{
	part_t p;
	u_int partent;
	u_char *soff, *eoff;
	int i;

	bzero(parttab, PARTS * sizeof(struct part));

	for (i = 0, p = parttab, partent = PART_ENT_1;
	     i < PARTS; i++, p++, partent += PART_ENT_SIZE) {
		p->active = (u_char) loadbyte(mbr + partent + PART_OFF_BOOT);
		p->sys = (u_char) loadbyte(mbr + partent + PART_OFF_SYS);
		soff = mbr + partent + PART_OFF_START;
		eoff = mbr + partent + PART_OFF_END;
		p->st = (u_int) loadbyte(soff + 2) |
		    (((u_int) loadbyte(soff + 1) & 0xc0) << 2);
		p->sh = (u_int) loadbyte(soff);
		p->ss = (u_int) loadbyte(soff + 1) & 0x3f;
		p->et = (u_int) loadbyte(eoff + 2) |
		    (((u_int) loadbyte(eoff + 1) & 0xc0) << 2);
		p->eh = (u_int) loadbyte(eoff);
		p->es = (u_int) loadbyte(eoff + 1) & 0x3f;
		p->off = (u_long) loaddword(mbr + partent + PART_OFF_OFF);
		p->size = (u_long) loaddword(mbr + partent + PART_OFF_SIZE);
	}
}

void write_parttab(part_t parttab, u_char * mbr)
{
	part_t p;
	u_int partent;
	int i;

	for (i = 0, p = parttab, partent = PART_ENT_1;
	     i < PARTS; i++, p++, partent += PART_ENT_SIZE) {
		storebyte((u_char) p->active, mbr + partent + PART_OFF_BOOT);
		storebyte((u_char) p->sh, mbr + partent + PART_OFF_START);
		storebyte(((u_char) ((p->st & 0x300) >> 2)) |
			  ((u_char) (p->ss & 0x3f)),
			  mbr + partent + PART_OFF_START + 1);
		storebyte((u_char) (p->st & 0xff),
			  mbr + partent + PART_OFF_START + 2);
		storebyte((u_char) p->sys, mbr + partent + PART_OFF_SYS);
		storebyte((u_char) p->eh, mbr + partent + PART_OFF_END);
		storebyte(((u_char) ((p->et & 0x300) >> 2)) |
			  ((u_char) (p->es & 0x3f)),
			  mbr + partent + PART_OFF_END + 1);
		storebyte((u_char) (p->et & 0xff),
			  mbr + partent + PART_OFF_END + 2);
		storedword(p->off, mbr + partent + PART_OFF_OFF);
		storedword(p->size, mbr + partent + PART_OFF_SIZE);
	}
}

static void dump_part(int partno, part_t p)
{
	kprintf("%c", (p->active ? '*' : ' '));
	kprintf("%3d  %4u  %3u     %2u  %4u  %3u     %2u %8u %12u ",
		partno, p->st, p->sh, p->ss, p->et, p->eh, p->es, p->off,
		p->size);
	switch (p->sys) {
	case PART_SYS_NONE:
		kprintf("unused");
		break;
	case PART_SYS_FAT12:
		kprintf("fat12");
		break;
	case PART_SYS_FAT16:
		kprintf("fat16");
		break;
	case PART_SYS_EXT:
		kprintf("extended");
		break;
	case PART_SYS_LARGE:
		kprintf("fat16 large");
		break;
	case PART_SYS_HPFS:
		kprintf("hpfs");
		break;
	case PART_SYS_FAT32:
		kprintf("fat32");
		break;
	case PART_SYS_UFS:
		kprintf("ufs");
		break;
	case PART_SYS_LINUX_SWAP:
		kprintf("linux swap");
		break;
	case PART_SYS_EXT2FS:
		kprintf("ext2fs");
		break;
	case PART_SYS_RRFS:
		kprintf("rrfs");
		break;
	default:
		kprintf("0x%02x", (u_char) p->sys);
	}
	kprintf("\n");
}

#define PART_HDR1                                                       \
    "     ------start------ -------end-------"
#define PART_HDR2                                                       \
    "part track head sector track head sector   offset         size type"

void dump_parttab(part_t parttab)
{
	part_t p;
	int i;

	kprintf("%s\n", PART_HDR1);
	kprintf("%s\n", PART_HDR2);
	for (i = 0, p = parttab; i < PARTS; i++, p++)
		dump_part(i, p);
}
