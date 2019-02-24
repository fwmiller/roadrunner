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

#ifndef __BOOT_PARAMS_H
#define __BOOT_PARAMS_H

#include <sys/types.h>

#define BP_DRV_HD	0x80

struct vbe_info {
    u_char signature[4];
    u_short version;
    u_long oem_string_ptr;
    u_char capabilities[4];
    u_long video_mode_ptr;
    u_short total_memory;
    u_char reserved[236];
} __attribute__ ((packed));

struct bootparams {
    /*
     * These elements are filled in by the boot program.  Do not change the
     * order or offset of these fields!
     */
    u_char drv;
    u_char cyl_lo;
    u_char sec_cyl_hi;
    u_char hd;
    u_char drvs;
    u_long offset;
    u_char reserved;
    struct vbe_info vbe;

    /* These elements are filled in during kernel initialization */
    int part;
} __attribute__ ((packed));

typedef struct bootparams *bootparams_t;

extern struct bootparams bootparams;

void get_boot_params();

#endif
