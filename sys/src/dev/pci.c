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

#include <bus/pci.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys.h>
#include <sys/mem.h>

/* Slots per PCI bus */
#define PCI_SLOTS				32

/* PCI function classes */
#define PCI_CLASS_PRE				0
#define PCI_CLASS_STORAGE			0x01
#define PCI_CLASS_NETWORK			0x02
#define PCI_CLASS_DISPLAY			0x03
#define PCI_CLASS_MULTIMEDIA			0x04
#define PCI_CLASS_MEMORY			0x05
#define PCI_CLASS_BRIDGE			0x06
#define PCI_CLASS_COMM				0x07
#define PCI_CLASS_BASE				0x08
#define PCI_CLASS_INPUT				0x09
#define PCI_CLASS_DOCK				0x0a
#define PCI_CLASS_CPU				0x0b
#define PCI_CLASS_SERIAL			0x0c
#define PCI_CLASS_WIRELESS			0x0d
#define PCI_CLASS_IIO				0x0e
#define PCI_CLASS_SATELLITE			0x0f
#define PCI_CLASS_SEC				0x10
#define PCI_CLASS_DSP				0x11

/* PCI storage function subclasses */
#define PCI_STORAGE_SCSI			0
#define PCI_STORAGE_IDE				1
#define PCI_STORAGE_FLOPPY			2
#define PCI_STORAGE_RAID			4

/* PCI network function subclasses */
#define PCI_NETWORK_ETHERNET			0
#define PCI_NETWORK_TOKEN			1
#define PCI_NETWORK_FDDI			2
#define PCI_NETWORK_ATM				3
#define PCI_NETWORK_ISDN			4

/* PCI display function subclasses */
#define PCI_DISPLAY_VGA				0
#define PCI_DISPLAY_XGA				1
#define PCI_DISPLAY_3D				2

/* PCI multimedia function subclasses */
#define PCI_MULTIMEDIA_VIDEO			0
#define PCI_MULTIMEDIA_AUDIO			1
#define PCI_MULTIMEDIA_TELEPHONY		2

/* PCI memory controller function subclasses */
#define PCI_MEMORY_RAM				0
#define PCI_MEMORY_FLASH			1

/* PCI bridge function subclasses */
#define PCI_BRIDGE_HOST				0
#define PCI_BRIDGE_ISA				1
#define PCI_BRIDGE_EISA				2
#define PCI_BRIDGE_MICROCHAN			3
#define PCI_BRIDGE_PCI				4
#define PCI_BRIDGE_PCMCIA			5
#define PCI_BRIDGE_NUBUS			6
#define PCI_BRIDGE_CARDBUS			7
#define PCI_BRIDGE_RACEWAY			8

/* PCI serial bus controller function subclasses */
#define PCI_SERIAL_IEEE1394			0
#define PCI_SERIAL_USB				3
#define PCI_SERIAL_FIBRE_CHANNEL		4

/* Name lengths */
#define PCI_VENDOR_NAME_LEN			18
#define PCI_CLASS_NAME_LEN			26
#define PCI_STORAGE_SUBCLASS_NAME_LEN		18
#define PCI_NETWORK_SUBCLASS_NAME_LEN		20
#define PCI_DISPLAY_SUBCLASS_NAME_LEN		14
#define PCI_MULTIMEDIA_SUBCLASS_NAME_LEN	20
#define PCI_MEMORY_SUBCLASS_NAME_LEN		16
#define PCI_BRIDGE_SUBCLASS_NAME_LEN		12
#define PCI_SERIAL_SUBCLASS_NAME_LEN		24

struct pci_vendor {
	u_short id;
	char name[PCI_VENDOR_NAME_LEN];
};

struct pci_class {
	u_short id;
	char name[PCI_CLASS_NAME_LEN];
};

struct pci_storage_subclass {
	u_short id;
	char name[PCI_STORAGE_SUBCLASS_NAME_LEN];
};

struct pci_network_subclass {
	u_short id;
	char name[PCI_NETWORK_SUBCLASS_NAME_LEN];
};

struct pci_display_subclass {
	u_short id;
	char name[PCI_DISPLAY_SUBCLASS_NAME_LEN];
};

struct pci_multimedia_subclass {
	u_short id;
	char name[PCI_MULTIMEDIA_SUBCLASS_NAME_LEN];
};

struct pci_memory_subclass {
	u_short id;
	char name[PCI_MEMORY_SUBCLASS_NAME_LEN];
};

struct pci_bridge_subclass {
	u_short id;
	char name[PCI_BRIDGE_SUBCLASS_NAME_LEN];
};

struct pci_serial_subclass {
	u_short id;
	char name[PCI_SERIAL_SUBCLASS_NAME_LEN];
};

struct pci_vendor pci_vendors[] = {
	{PCI_VENDOR_COMPAQ, "Compaq"},
	{PCI_VENDOR_NCR, "NCR"},
	{PCI_VENDOR_ATI, "ATI"},
	{PCI_VENDOR_VLSI, "VLSI"},
	{PCI_VENDOR_TSENG, "Tseng"},
	{PCI_VENDOR_WEITEK, "Weitek"},
	{PCI_VENDOR_DEC, "DEC"},
	{PCI_VENDOR_CIRRUS, "Cirrus Logic"},
	{PCI_VENDOR_IBM, "IBM"},
	{PCI_VENDOR_AMD, "AMD"},
	{PCI_VENDOR_TRIDENT, "Trident"},
	{PCI_VENDOR_MATROX, "Matrox"},
	{PCI_VENDOR_NEC, "NEC"},
	{PCI_VENDOR_HP, "HP"},
	{PCI_VENDOR_BUSLOGIC, "Buslogic"},
	{PCI_VENDOR_TI, "Texas Instruments"},
	{PCI_VENDOR_MOTOROLA, "Motorola"},
	{PCI_VENDOR_NUMBER9, "Number 9"},
	{PCI_VENDOR_APPLE, "Apple"},
	{PCI_VENDOR_CYRIX, "Cyrix"},
	{PCI_VENDOR_SUN, "Sun"},
	{PCI_VENDOR_3COM, "3Com"},
	{PCI_VENDOR_ACER, "Acer Labs"},
	{PCI_VENDOR_MITSUBISHI, "Mitsubishi"},
	{PCI_VENDOR_NVIDIA, "Nvidia"},
	{PCI_VENDOR_FORE, "Fore"},
	{PCI_VENDOR_PHILLIPS, "Phillips"},
	{PCI_VENDOR_RENDITION, "Rendition"},
	{PCI_VENDOR_TOSHIBA, "Toshiba"},
	{PCI_VENDOR_ENSONIQ, "Ensoniq"},
	{PCI_VENDOR_ROCKWELL, "Rockwell"},
	{PCI_VENDOR_NETGEAR, "Netgear"},
	{PCI_VENDOR_VMWARE, "VMware"},
	{PCI_VENDOR_S3, "S3"},
	{PCI_VENDOR_INTEL, "Intel"},
	{PCI_VENDOR_ADAPTEC, "Adaptec"},
	{PCI_VENDOR_ADAPTEC2, "Adaptec"}
};

struct pci_class pci_classes[] = {
	{PCI_CLASS_PRE, "pre ver 2.0"},
	{PCI_CLASS_STORAGE, "mass storage controller"},
	{PCI_CLASS_NETWORK, "network interface"},
	{PCI_CLASS_DISPLAY, "display controller"},
	{PCI_CLASS_MULTIMEDIA, "multimedia controller"},
	{PCI_CLASS_MEMORY, "memory controller"},
	{PCI_CLASS_BRIDGE, "bridge controller"},
	{PCI_CLASS_COMM, "communications controller"},
	{PCI_CLASS_BASE, "base system peripheral"},
	{PCI_CLASS_INPUT, "input controller"},
	{PCI_CLASS_DOCK, "docking station"},
	{PCI_CLASS_CPU, "processor"},
	{PCI_CLASS_SERIAL, "serial bus controller"},
	{PCI_CLASS_WIRELESS, "wireless interface"},
	{PCI_CLASS_IIO, "intelligent I/O controller"},
	{PCI_CLASS_SATELLITE, "satellite interface"},
	{PCI_CLASS_SEC, "encryption/decryption"},
	{PCI_CLASS_DSP, "digital signal processor"}
};

struct pci_storage_subclass pci_storage_subclasses[] = {
	{PCI_STORAGE_SCSI, "SCSI controller"},
	{PCI_STORAGE_IDE, "IDE controller"},
	{PCI_STORAGE_FLOPPY, "floppy controller"},
	{PCI_STORAGE_RAID, "RAID controller"}
};

struct pci_network_subclass pci_network_subclasses[] = {
	{PCI_NETWORK_ETHERNET, "Ethernet interface"},
	{PCI_NETWORK_TOKEN, "Token Ring interface"},
	{PCI_NETWORK_FDDI, "FDDI interface"},
	{PCI_NETWORK_ATM, "ATM interface"},
	{PCI_NETWORK_ISDN, "ISDN interface"}
};

struct pci_display_subclass pci_display_subclasses[] = {
	{PCI_DISPLAY_VGA, "VGA controller"},
	{PCI_DISPLAY_XGA, "XGA controller"},
	{PCI_DISPLAY_3D, "3D controller"}
};

struct pci_multimedia_subclass pci_multimedia_subclasses[] = {
	{PCI_MULTIMEDIA_VIDEO, "video controller"},
	{PCI_MULTIMEDIA_AUDIO, "audio controller"},
	{PCI_MULTIMEDIA_TELEPHONY, "telephony controller"}
};

struct pci_memory_subclass pci_memory_subclasses[] = {
	{PCI_MEMORY_RAM, "RAM controller"},
	{PCI_MEMORY_FLASH, "Flash controller"}
};

struct pci_bridge_subclass pci_bridge_subclasses[] = {
	{PCI_BRIDGE_HOST, "host"},
	{PCI_BRIDGE_ISA, "ISA"},
	{PCI_BRIDGE_EISA, "EISA"},
	{PCI_BRIDGE_MICROCHAN, "Microchannel"},
	{PCI_BRIDGE_PCI, "PCI"},
	{PCI_BRIDGE_PCMCIA, "PCMCIA"},
	{PCI_BRIDGE_NUBUS, "Nubus"},
	{PCI_BRIDGE_CARDBUS, "Cardbus"},
	{PCI_BRIDGE_RACEWAY, "Raceway"}
};

struct pci_serial_subclass pci_serial_subclasses[] = {
	{PCI_SERIAL_IEEE1394, "IEEE 1394 controller"},
	{PCI_SERIAL_USB, "USB controller"},
	{PCI_SERIAL_FIBRE_CHANNEL, "Fibre Channel controller"}
};

static int pcifuncs = 0;
static int pcifunc = 0;
static pci_func_t pcitab;

#if _DEBUG_PCI
static int vendors = (sizeof(pci_vendors) / sizeof(struct pci_vendor));
static int classes = (sizeof(pci_classes) / sizeof(struct pci_class));
static int storagesubclasses =
    (sizeof(pci_storage_subclasses) / sizeof(struct pci_storage_subclass));
static int networksubclasses =
    (sizeof(pci_network_subclasses) / sizeof(struct pci_network_subclass));
static int displaysubclasses =
    (sizeof(pci_display_subclasses) / sizeof(struct pci_display_subclass));
static int multimediasubclasses =
    (sizeof(pci_multimedia_subclasses) /
     sizeof(struct pci_multimedia_subclass));
static int memorysubclasses =
    (sizeof(pci_memory_subclasses) / sizeof(struct pci_memory_subclass));
static int bridgesubclasses =
    (sizeof(pci_bridge_subclasses) / sizeof(struct pci_bridge_subclass));
static int serialsubclasses =
    (sizeof(pci_serial_subclasses) / sizeof(struct pci_serial_subclass));
#endif

u_long pci_config_read(int bus, int dev, int func, int dword)
{
	outl(PCI_CONFIG_ADDR, ((u_long) 0x80000000 | (bus << 16) |
			       (dev << 11) | (func << 8) | (dword << 2)));
	return inl(PCI_CONFIG_DATA);
}

void pci_config_write(int bus, int dev, int func, int dword, u_long val)
{
	outl(PCI_CONFIG_ADDR, ((u_long) 0x80000000 | (bus << 16) |
			       (dev << 11) | (func << 8) | (dword << 2)));
	outl(PCI_CONFIG_DATA, val);
}

void pci_busmaster_enable(int bus, int dev, int func)
{
	u_long cmdstat;

	cmdstat = pci_config_read(bus, dev, func, PCI_CONFIG_CMD_STAT);
	cmdstat |= 0x04;
	pci_config_write(bus, dev, func, PCI_CONFIG_CMD_STAT, cmdstat);
}

static int pci_device_count(int bus)
{
	u_long vendorid;
	int devs = 0, dev;

	for (dev = 0; dev < PCI_SLOTS; dev++) {
		vendorid =
		    pci_config_read(bus, dev, 0, PCI_CONFIG_VENDOR) & 0xffff;
		if (vendorid < 0xffff)
			devs++;
	}
#if _DEBUG_PCI
	if (devs > 0)
		kprintf("pci_device_count: %d devices on pci bus %d\n", devs,
			bus);
#endif
	return devs;
}

#if _DEBUG_PCI
static void dump_pcivendor(u_short vendorid)
{
	int i;

	for (i = 0; i < vendors; i++)
		if (pci_vendors[i].id == vendorid) {
			kprintf("%s", pci_vendors[i].name);
			break;
		}
	if (i == vendors)
		kprintf("vendor id %04x", vendorid);
}

static void dump_pciclass(u_short class, u_short subclass)
{
	int i, j;

	for (i = 0; i < classes; i++)
		if (pci_classes[i].id == class) {
			switch (class) {
			case PCI_CLASS_STORAGE:
				for (j = 0; j < storagesubclasses; j++)
					if (pci_storage_subclasses[j].id ==
					    subclass) {
						kprintf(" %s",
							pci_storage_subclasses
							[j].name);
						break;
					}
				if (j == storagesubclasses)
					kprintf(" %s", pci_classes[i].name);
				break;

			case PCI_CLASS_NETWORK:
				for (j = 0; j < networksubclasses; j++)
					if (pci_network_subclasses[j].id ==
					    subclass) {
						kprintf(" %s",
							pci_network_subclasses
							[j].name);
						break;
					}
				if (j == networksubclasses)
					kprintf(" %s", pci_classes[i].name);
				break;

			case PCI_CLASS_DISPLAY:
				for (j = 0; j < displaysubclasses; j++)
					if (pci_display_subclasses[j].id ==
					    subclass) {
						kprintf(" %s",
							pci_display_subclasses
							[j].name);
						break;
					}
				if (j == displaysubclasses)
					kprintf(" %s", pci_classes[i].name);
				break;

			case PCI_CLASS_MULTIMEDIA:
				for (j = 0; j < multimediasubclasses; j++)
					if (pci_multimedia_subclasses[j].id ==
					    subclass) {
						kprintf(" %s",
							pci_multimedia_subclasses
							[j].name);
						break;
					}
				if (j == multimediasubclasses)
					kprintf(" %s", pci_classes[i].name);
				break;

			case PCI_CLASS_MEMORY:
				for (j = 0; j < memorysubclasses; j++)
					if (pci_memory_subclasses[j].id ==
					    subclass) {
						kprintf(" %s",
							pci_memory_subclasses
							[j].name);
						break;
					}
				if (j == memorysubclasses)
					kprintf(" %s", pci_classes[i].name);
				break;

			case PCI_CLASS_BRIDGE:
				for (j = 0; j < bridgesubclasses; j++)
					if (pci_bridge_subclasses[j].id ==
					    subclass) {
						kprintf(" %s",
							pci_bridge_subclasses
							[j].name);
						break;
					}
				kprintf(" %s", pci_classes[i].name);
				break;

			case PCI_CLASS_SERIAL:
				for (j = 0; j < serialsubclasses; j++)
					if (pci_serial_subclasses[j].id ==
					    subclass) {
						kprintf(" %s",
							pci_serial_subclasses
							[j].name);
						break;
					}
				if (j == serialsubclasses)
					kprintf(" %s", pci_classes[i].name);
				break;

			default:
				kprintf(" %s", pci_classes[i].name);
			}
			break;
		}
	if (i == classes)
		kprintf(" class code %02x", class);
}

static void dump_pci_func(int slot, pci_func_t func)
{
	kprintf("dump_pci_func: ");
	kprintf("<%d,%d,%d>", func->bus, func->dev, func->func);
	kprintf(" vendor %04x device %04x iobase 0x%x irq %d\n",
		func->vendorid, func->deviceid, func->iobase, func->irq);
}

static void dump_pcitab()
{
	int i;

	for (i = 0; i < pcifuncs; i++)
		dump_pci_func(i, &(pcitab[i]));
}
#endif

static void pci_scan_bus(int bus)
{
	u_long dword, iobase;
	u_short vendorid, deviceid;
	u_char class, subclass, intrpin;
	int dev, intrline, i;

	for (dev = 0; dev < PCI_SLOTS; dev++) {
		/* Function and vendor ids */
		dword = pci_config_read(bus, dev, 0, PCI_CONFIG_VENDOR);
		vendorid = dword & 0xffff;
		deviceid = dword >> 16;

		if (vendorid < 0xffff) {
#if _DEBUG_PCI
			kprintf("pci_scan_bus: ");
			dump_pcivendor((u_short) vendorid);
#endif
			/* Function class code */
			dword =
			    pci_config_read(bus, dev, 0, PCI_CONFIG_CLASS_REV);
			class = dword >> 24;
			subclass = (dword >> 16) & 0xff;
#if _DEBUG_PCI
			dump_pciclass(class, subclass);
#endif
			/* Function iobase addresses */
			for (iobase = 0, i = 0; i < 6; i++) {
				dword = pci_config_read(bus, dev, 0,
							PCI_CONFIG_BASE_ADDR_0 +
							i);
				if (dword & 0x01) {
					iobase = dword & 0xfffffffc;
#if _DEBUG_PCI
					kprintf(" iobase 0x%x", iobase);
#endif
				}
			}
			/* Function interrupt line */
			dword = pci_config_read(bus, dev, 0, PCI_CONFIG_INTR);
			intrpin = (u_char) (dword >> 8) & 0xff;
			intrline = dword & 0xff;
#if _DEBUG_PCI
			if (intrpin > 0 && intrpin < 5 && intrline < 32)
				kprintf(" irq %d", intrline);
			kprintf(" \n");
#endif

			/* Fill in pci device table entry */
			pcitab[pcifunc].bus = bus;
			pcitab[pcifunc].dev = dev;
			pcitab[pcifunc].func = 0;
			pcitab[pcifunc].vendorid = (u_short) vendorid;
			pcitab[pcifunc].deviceid = deviceid;
			pcitab[pcifunc].iobase = iobase;
			pcitab[pcifunc].irq = intrline;

			pcifunc++;
		}
	}
}

int pci_init()
{
	int devices[4], i;

	for (pcifuncs = 0, i = 0; i < 4; i++) {
		devices[i] = pci_device_count(i);
		pcifuncs += devices[i];
	}
#if _DEBUG_PCI
	kprintf("pci_init: %d pci functions\n", pcifuncs);
#endif
	pcitab = (pci_func_t) kmalloc(pcifuncs * sizeof(struct pci_func));
	bzero(pcitab, pcifuncs * sizeof(struct pci_func));

	for (i = 0; i < 4; i++)
		if (devices[i] > 0)
			pci_scan_bus(i);
#if _DEBUG_PCI
	kprintf("pci_init: pci device table\n");
	dump_pcitab();
#endif
	return 0;
}

pci_func_t pci_lookup(u_short vendorid, u_short deviceid)
{
	int i;

	for (i = 0; i < pcifuncs; i++)
		if (pcitab[i].vendorid == vendorid
		    && pcitab[i].deviceid == deviceid)
			break;
	if (i == pcifuncs)
		return NULL;
	return &(pcitab[i]);
}
