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

#ifndef __PCI_H
#define __PCI_H

#if _KERNEL

#include <sys/types.h>

/* Ports for access to PCI config space */
#define PCI_CONFIG_ADDR			0xcf8
#define PCI_CONFIG_DATA			0xcfc

/* PCI config space register offsets */
#define PCI_CONFIG_VENDOR		0
#define PCI_CONFIG_CMD_STAT		1
#define PCI_CONFIG_CLASS_REV		2
#define PCI_CONFIG_HDR_TYPE		3
#define PCI_CONFIG_BASE_ADDR_0		4
#define PCI_CONFIG_BASE_ADDR_1		5
#define PCI_CONFIG_BASE_ADDR_2		6
#define PCI_CONFIG_BASE_ADDR_3		7
#define PCI_CONFIG_BASE_ADDR_4		8
#define PCI_CONFIG_BASE_ADDR_5		9
#define PCI_CONFIG_CIS			10
#define PCI_CONFIG_SUBSYSTEM		11
#define PCI_CONFIG_ROM			12
#define PCI_CONFIG_CAPABILITIES		13
#define PCI_CONFIG_INTR			15

/* PCI vendor IDs */
#define PCI_VENDOR_COMPAQ		0x0e11
#define PCI_VENDOR_NCR			0x1000
#define PCI_VENDOR_ATI			0x1002
#define PCI_VENDOR_VLSI			0x1004
#define PCI_VENDOR_TSENG		0x100c
#define PCI_VENDOR_WEITEK		0x100e
#define PCI_VENDOR_DEC			0x1011
#define PCI_VENDOR_CIRRUS		0x1013
#define PCI_VENDOR_IBM			0x1014
#define PCI_VENDOR_AMD			0x1022
#define PCI_VENDOR_TRIDENT		0x1023
#define PCI_VENDOR_MATROX		0x102b
#define PCI_VENDOR_NEC			0x1033
#define PCI_VENDOR_HP			0x103c
#define PCI_VENDOR_BUSLOGIC		0x104b
#define PCI_VENDOR_TI			0x104c
#define PCI_VENDOR_MOTOROLA		0x1057
#define PCI_VENDOR_NUMBER9		0x105d
#define PCI_VENDOR_APPLE		0x106b
#define PCI_VENDOR_CYRIX		0x1078
#define PCI_VENDOR_SUN			0x108e
#define PCI_VENDOR_3COM			0x10b7
#define PCI_VENDOR_ACER			0x10b9
#define PCI_VENDOR_MITSUBISHI		0x10ba
#define PCI_VENDOR_NVIDIA		0x10de
#define PCI_VENDOR_FORE			0x1127
#define PCI_VENDOR_PHILLIPS		0x1131
#define PCI_VENDOR_RENDITION		0x1163
#define PCI_VENDOR_TOSHIBA		0x1179
#define PCI_VENDOR_ENSONIQ		0x1274
#define PCI_VENDOR_ROCKWELL		0x127a
#define PCI_VENDOR_NETGEAR		0x1385
#define PCI_VENDOR_VMWARE		0x15ad
#define PCI_VENDOR_S3			0x5333
#define PCI_VENDOR_INTEL		0x8086
#define PCI_VENDOR_ADAPTEC		0x9004
#define PCI_VENDOR_ADAPTEC2		0x9005

struct pci_func {
    int bus, dev, func;		       /* Function logical address */
    u_short vendorid;		       /* Vendor id */
    u_short deviceid;		       /* Device id */
    u_long iobase;		       /* I/O registers base addr */
    int irq;			       /* Interrupt number */
};

typedef struct pci_func *pci_func_t;

u_long pci_config_read(int bus, int dev, int func, int dword);
void pci_config_write(int bus, int dev, int func, int dword, u_long val);
void pci_busmaster_enable(int bus, int dev, int func);
pci_func_t pci_lookup(u_short vendor, u_short device);
int pci_init();

#endif				/* _KERNEL */

#endif
