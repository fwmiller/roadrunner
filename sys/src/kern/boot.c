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
#include <sys/boot.h>

struct bootparams bootparams;

#if _DEBUG
static u_long
real2protptr(u_long ptr)
{
    return ((ptr & 0xf0000000) >> 12) | (ptr & 0xffff);
}
#endif

void
get_boot_params()
{
    bcopy((char *) 0x1000, &bootparams, sizeof(struct bootparams));

#if _DEBUG
    /* Fixed disk information */
    kprintf("get_boot_params: boot drive %s",
	    bootparams.drv & 0x80 ? "hard disk" : "floppy disk");
    if (bootparams.drv & 0x80)
	kprintf(" partition offset %u", (u_int) bootparams.offset);
    kprintf("\n");

    kprintf("get_boot_params: ");
    kprintf("BIOS geometry %u trks %u hds %u sec/trk\n",
	    (u_int) ((((bootparams.sec_cyl_hi & 0xc0) << 2) |
		      bootparams.cyl_lo) + 1),
	    (u_int) (bootparams.hd + 1),
	    (u_int) (bootparams.sec_cyl_hi & 0x3f));

    /* VBE controller information */
    if (strncmp((char *) bootparams.vbe.signature, "VESA", 4) == 0) {
	kprintf("get_boot_params: VESA version %d.%d\n",
		bcd2int(bootparams.vbe.version >> 8),
		bcd2int(bootparams.vbe.version));
	kprintf("get_boot_params: VESA OEM string: %s\n",
		(char *) real2protptr(bootparams.vbe.oem_string_ptr));
	kprintf("get_boot_params: ");
	kprintf("VESA video memory %d 64 Kbyte banks\n",
		bootparams.vbe.total_memory);
    }
#endif
}
