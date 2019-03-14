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
#include <dev.h>
#include <dev/ata.h>
#include <dev/cons.h>
#include <dev/fd.h>
#include <dev/hd.h>
#include <dev/kbd.h>
#include <dev/uart.h>
#include <event.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fs/devfs.h>
#include <fs/ramfs.h>
#include <fs/rrfs.h>
#include <fs/sysfs.h>
#include <sys/boot.h>
#include <sys/intr.h>
#include <sys/mem.h>
#include <sys/timer.h>
#include <sys/utsname.h>

extern char *initkstk;

void cpuid();

void os()
{
	struct utsname name;
	struct dev_ops devops;
	struct fsops fsops;
	fsops_t sysfsops, devfsops, ramfsops, rrfsops;
	fs_t sysfs, devfs, ramfs, rrfs;
	file_t fdin = NULL, fdout = NULL, fderr = NULL;

	intr_init();
#if 0
	clear_screen();
#endif
	uart_init(NULL);

	uname(&name);
	kprintf("%s %s.%s %s\n",
		name.sysname, name.version, name.release, COPYRIGHT);

	get_boot_params();
	cpuid();
	mem_init();
	vm_init();
	isrtab_init();
	eventtab_init();
	proc_sysinit();
	time_init();
	timer_init();
	kbd_init(NULL);
	devtab_init();
	blkpool_init(PAGE_SIZE, BLKS);
	bufpool_init(BUFS);
	fstab_init();

	/* Setup context for init process */
	proctab[0].state = PS_RUN;
	strcpy(proctab[0].cwd, "/");

	/* Init process never leaves kernel mode */
	ktssinit(proctab[0].context.tss, (u_long) initkstk, STACK_SIZE);

	/* Attach a set of page tables to init process */
	proctab[0].context.ptrec = pt_pop();
	proctab[0].context.tss->cr3 = (u_long) proctab[0].context.ptrec->pd;

	/* Setup page tables for init process */
	vm_map_init((pt_t) proctab[0].context.tss->cr3);
	vm_kmap((pt_t) proctab[0].context.tss->cr3);

	/* Turn paging on */
	vm_enable(proctab[0].context.tss->cr3);

	/* Load task register with first TSS descriptor */
	ltr((u_long) tssdesctab - (u_long) gdt);

	/* Init process is made currently executing process */
	current = &(proctab[0]);

	kprintf("real mem %d\n", (int)memsize);
	kprintf("kernel size %d\n", (int)kernsize);
	{
		region_t r;
		size_t freemem = 0;

		for (r = freelist; r != NULL; r = r->next)
			freemem += r->len;
		kprintf("avail mem %d\n", (int)freemem);
	}
#if _PCI
	/* Scan for PCI devices */
	pci_init();
#endif

	/* Install keyboard device */
	devops.init = kbd_init;
	devops.shut = kbd_shut;
	devops.ioctl = kbd_ioctl;
	devops.specific.char_ops.get = kbd_get;
	devops.specific.char_ops.put = NULL;
	dev_inst("kbd", DEV_TYPE_CHAR, &devops, NULL);

	/* Install console display device */
	devops.init = cons_init;
	devops.shut = cons_shut;
	devops.ioctl = cons_ioctl;
	devops.specific.char_ops.get = NULL;
	devops.specific.char_ops.put = cons_put;
	dev_inst("cons", DEV_TYPE_CHAR, &devops, NULL);

	/* Install uart  device */
	devops.init = uart_init;
	devops.shut = uart_shut;
	devops.ioctl = uart_ioctl;
	devops.specific.char_ops.get = uart_get;
	devops.specific.char_ops.put = uart_put;
	dev_inst("uart", DEV_TYPE_CHAR, &devops, NULL);

	/* Install floppy disk device */
	devops.init = fd_init;
	devops.shut = fd_shut;
	devops.ioctl = fd_ioctl;
	devops.specific.blk_ops.read = fd_read;
	devops.specific.blk_ops.write = fd_write;
	dev_inst("fd", DEV_TYPE_BLK, &devops, NULL);
	dev_init("fd");

	/* Install hard disk support */
	hdtab_init();
	ata_init();

	/* Install and initialize /sys file system */
	strcpy(fsops.name, "sysfs");
	fsops.init = sysfs_init;
	fsops.shut = sysfs_shut;
	fsops.mount = sysfs_mount;
	fsops.unmount = sysfs_unmount;
	fsops.open = sysfile_open;
	fsops.close = sysfile_close;
	fsops.ioctl = sysfile_ioctl;
	fsops.read = sysfile_read;
	fsops.write = sysfile_write;
	fsops.attr = sysfile_attr;
	fsops.readdir = sysfile_readdir;
	fsops.unlink = sysfile_unlink;
	sysfsops = fsops_inst(&fsops);
	fsops_init(sysfsops);

	/* Install and initialize device file system */
	strcpy(fsops.name, "devfs");
	fsops.init = devfs_init;
	fsops.shut = devfs_shut;
	fsops.mount = devfs_mount;
	fsops.unmount = devfs_unmount;
	fsops.open = devfile_open;
	fsops.close = devfile_close;
	fsops.ioctl = devfile_ioctl;
	fsops.read = devfile_read;
	fsops.write = devfile_write;
	fsops.attr = devfile_attr;
	fsops.readdir = devfile_readdir;
	fsops.unlink = devfile_unlink;
	devfsops = fsops_inst(&fsops);
	fsops_init(devfsops);

	/* Install and initialize ramfs file system */
	strcpy(fsops.name, "ramfs");
	fsops.init = ramfs_init;
	fsops.shut = ramfs_shut;
	fsops.mount = ramfs_mount;
	fsops.unmount = ramfs_unmount;
	fsops.open = ramfile_open;
	fsops.close = ramfile_close;
	fsops.ioctl = ramfile_ioctl;
	fsops.read = ramfile_read;
	fsops.write = ramfile_write;
	fsops.attr = ramfile_attr;
	fsops.readdir = ramfile_readdir;
	fsops.unlink = ramfile_unlink;
	ramfsops = fsops_inst(&fsops);
	fsops_init(ramfsops);

	/* Install and initialize rrfs file system */
	strcpy(fsops.name, "rrfs");
	fsops.init = rrfs_init;
	fsops.shut = rrfs_shut;
	fsops.mount = rrfs_mount;
	fsops.unmount = rrfs_unmount;
	fsops.open = rrfile_open;
	fsops.close = rrfile_close;
	fsops.ioctl = rrfile_ioctl;
	fsops.read = rrfile_read;
	fsops.write = rrfile_write;
	fsops.attr = rrfile_attr;
	fsops.readdir = rrfile_readdir;
	fsops.unlink = rrfile_unlink;
	rrfsops = fsops_inst(&fsops);
	fsops_init(rrfsops);

	/* Mount /dev and /sys file systems */
	fs_mount(sysfsops, "/sys", (-1), &sysfs);
	fs_mount(devfsops, "/dev", (-1), &devfs);

	/* Mount root file system */
	if ((bootparams.drv & BP_DRV_HD) == BP_DRV_HD) {
		char device[16];

		bzero(device, 16);
		ata_get_boot_device(bootparams.drv, device);
		kprintf("root file system on /dev/%s\n", device);
		fs_mount(rrfsops, "/", dev_open(device), &rrfs);

	} else {
		kprintf("root file system on /dev/fd\n");
		fs_mount(rrfsops, "/", dev_open("fd"), &rrfs);
	}
	/* Setup stdpaths for init process */
	file_open("/dev/kbd", O_RDONLY, &fdin);
	file_open("/dev/cons", O_WRONLY, &fdout);
	file_open("/dev/cons", O_WRONLY, &fderr);
	proctab[0].fd[PFD_STDIN] = fdin->slot;
	proctab[0].fd[PFD_STDOUT] = fdout->slot;
	proctab[0].fd[PFD_STDERR] = fderr->slot;

	/* Start command shells for console */
	proc_exec("/bin/sh", 0, NULL);

	for (;;) ;
	/* Not reached */
}
