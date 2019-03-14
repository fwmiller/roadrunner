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

#include <dev/ata.h>
#include <fs/rrfs.h>
#include <stdlib.h>
#include <string.h>
#include <sys.h>
#include <sys/boot.h>
#include <sys/config.h>
#include <sys/elf.h>
#include <sys/intr.h>

/* 
 * A set of boot parameters are passed from the first stage boot program
 * at this following location
 */
static bootparams_t params = (bootparams_t) 0x1000;

/* 
 * A place where the boot params can be saved during boot2 execution.
 * These parameters will be copied back to params before starting the
 * kernel.
 */
static bootparams_t saved_params;

/* 
 * The boot2 execution stack starts at 0x9fff8 and grows downwards.  boot2
 * should not need more than a couple of pages so temporary heap management
 * starts at 0x9e000.
 */
static char *heap = (char *)0x9e000;

static char *mbrbuf;
static char *fatbuf;
static char *clustbuf;
static char *addr;
static mbr_t mbr;
static u_int filepos = 0;
static u_int offset = 0;
static u_int maxblkno;
static u_int clust;
static int clustsize;
static int clustpos;
static Elf32_Ehdr *ehdr;
static Elf32_Phdr *phdr;

extern atap_t atap_boot;

void printf(char *fmt, ...);
void gateA20();
void intrinit();
int intrunmask(int intno);
void clear();
void ata_readblk(atad_t atad, u_long blkno, char *b);
void ata_set_boot_device(u_char drv);
void fd_init();
int fd_read(u_int blkno, char *b);
void halt();
void _start();

void *malloc(size_t size)
{
	size = ALIGN(size, PAGE_SIZE);
	heap -= size;
	return (void *)heap;
}

static void readsector(u_int blkno, char *buf)
{
	if (offset > 0)
		ata_readblk(atap_boot->atad, atap_boot->offset + blkno, buf);
	else {
		int result = fd_read(blkno, buf);

		if (result < 0) {
			printf("readsector: failed\n");
			halt();
		}
	}
}

static void readclust(char *buf)
{
	u_int blkno;
	int i;

	blkno = mbr->params.bootsectors +
	    2 * mbr->params.fatsectors + clust * mbr->params.sectorsperclust;

	if (blkno >= (maxblkno - mbr->params.sectorsperclust)) {
		printf("readclust: illegal blkno\n");
		halt();
	}
	for (i = 0; i < mbr->params.sectorsperclust; i++)
		readsector(blkno + i, buf + i * SECTOR_SIZE);
}

static u_int nextclust(mbr_t mbr, char *fat, u_int clust)
{
	u_int c;

	c = ((u_int *) fat)[clust];
	if (c < mbr->params.clusters)
		return c;
	return FAT_CHAIN_END;
}

static void read(int size)
{
	/* Start with any data left over from the last cluster read */
	if (clustpos < clustsize) {
		if (size < clustsize - clustpos) {
			/* 
			 * The amount of data left in the last cluster read is less than
			 * the requested size
			 */
			if (addr != 0) {
				bcopy(clustbuf + clustpos, addr, size);
				addr += size;
			}
			filepos += size;
			clustpos += size;
			return;
		} else {
			/* The requested size will drain the last cluster read */
			int len = clustsize - clustpos;

			if (addr != 0) {
				bcopy(clustbuf + clustpos, addr, len);
				addr += len;
			}
			size -= len;
			filepos += len;
			clustpos = clustsize;
		}
	}
	/* Read in new clusters as necessary */
	while (size > 0) {
		readclust(clustbuf);
		clustpos = 0;
		clust = nextclust(mbr, fatbuf, clust);
		if (size < clustsize) {
			if (addr != 0) {
				bcopy(clustbuf, addr, size);
				addr += size;
			}
			filepos += size;
			clustpos += size;
			break;
		} else {
			if (addr != 0) {
				bcopy(clustbuf, addr, clustsize);
				addr += clustsize;
			}
			size -= clustsize;
			filepos += clustsize;
			clustpos = clustsize;
		}
	}
}

#if _DEBUG
static void dumpmbr()
{
	printf("dumpmbr: %u trks  %u hds %u sec/trk %u sectors\n",
	       (u_int) mbr->params.tracks,
	       (u_int) mbr->params.heads,
	       (u_int) mbr->params.sectorspertrack,
	       (u_int) mbr->params.sectors);
	printf("dumpmbr: %u sec/fat %u sec/clust %u clusters\n",
	       (u_int) mbr->params.fatsectors,
	       (u_int) mbr->params.sectorsperclust,
	       (u_int) mbr->params.clusters);
}
#endif

void boot()
{
	direntry_t de;
	int i;

	gateA20();
	intrinit();
	clear();

	/* Save boot parameters */
	saved_params = (bootparams_t) malloc(sizeof(struct bootparams));
	bcopy(params, saved_params, sizeof(struct bootparams));

	if (params->drv & BP_DRV_HD) {
		offset = params->offset;
		ata_init();
		ata_set_boot_device(params->drv);
	} else {
		offset = 0;
		fd_init();
		intrunmask(INTR_FD);
	}
#if _DEBUG
	if (params->drv & 0x80)
		printf("boot: part offset %u\n", (u_int) params->offset);
#endif
	enable;

	/* Read rrfs file system mbr */
	mbrbuf = (char *)malloc(SECTOR_SIZE);
	readsector(0, mbrbuf);
	mbr = (mbr_t) mbrbuf;
	if (mbr->params.bytespersector != SECTOR_SIZE) {
		printf("boot: file system missing\n");
		halt();
	}
#if _DEBUG
	dumpmbr();
#endif

	clustsize = mbr->params.sectorsperclust * SECTOR_SIZE;
	clustpos = clustsize;
	maxblkno = mbr->params.bootsectors +
	    2 * mbr->params.fatsectors +
	    mbr->params.clusters * mbr->params.sectorsperclust;

	/* Read rrfs file system fat */
	fatbuf = (char *)malloc(mbr->params.fatsectors * SECTOR_SIZE);
	for (i = 0; i < mbr->params.fatsectors; i++)
		readsector(mbr->params.bootsectors + i,
			   fatbuf + i * SECTOR_SIZE);

	/* Read first cluster of root directory */
	clustbuf = (char *)malloc(mbr->params.sectorsperclust * SECTOR_SIZE);
	clust = 0;
	readclust(clustbuf);

	/* Search cluster for kernel file */
	for (i = 0, de = (direntry_t) clustbuf;
	     i < clustsize; i += DE_SIZE, de = (direntry_t) (clustbuf + i))
		if (strcmp(de->name, "kernel") == 0)
			break;
	if (i >= clustsize) {
		printf("boot: could not find kernel\n");
		halt();
	}
	/* Get kernel file start cluster */
	clust = (u_int) de->start;

	/* Read ELF file header information and do some checks */
	ehdr = (Elf32_Ehdr *) malloc(sizeof(Elf32_Ehdr));
	addr = (char *)ehdr;
	read(sizeof(Elf32_Ehdr));
	if (ehdr->e_ident[EI_MAG0] != ELFMAG0 ||
	    ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
	    ehdr->e_ident[EI_MAG2] != ELFMAG2 ||
	    ehdr->e_ident[EI_MAG3] != ELFMAG3 ||
	    ehdr->e_ident[EI_CLASS] != ELFCLASS32 ||
	    ehdr->e_ident[EI_DATA] != ELFDATA2LSB ||
	    ehdr->e_type != ET_EXEC || ehdr->e_machine != EM_386) {
		printf("boot: bad kernel file\n");
		halt();
	}
	/* Read ELF program headers */
	phdr = (Elf32_Phdr *) malloc(ehdr->e_phnum * sizeof(Elf32_Phdr));
	addr = (char *)phdr;
	read(ehdr->e_phnum * sizeof(Elf32_Phdr));

	for (i = 0; i < ehdr->e_phnum; i++) {
		/* Loadable segments only */
		if (phdr[i].p_type != PT_LOAD)
			continue;

		if (!(phdr[i].p_flags & PF_W))
			printf("text=0x%x ", (u_int) phdr[i].p_filesz);
		else {
			printf("data=0x%x", (u_int) phdr[i].p_filesz);
			if (phdr[i].p_filesz < phdr[i].p_memsz)
				printf("+0x%x",
				       (u_int) (phdr[i].p_memsz -
						phdr[i].p_filesz));
			printf(" ");
		}
#if _DEBUG
		printf("boot: vaddr 0x%x memsz 0x%x filesz 0x%x align %d\n",
		       (u_int) phdr[i].p_vaddr,
		       (u_int) phdr[i].p_memsz,
		       (u_int) phdr[i].p_filesz, (int)phdr[i].p_align);
#endif
		if (filepos < phdr[i].p_offset) {
			addr = 0;
			read(phdr[i].p_offset - filepos);
		}
		addr = (char *)phdr[i].p_vaddr;
		bzero(addr, phdr[i].p_memsz);
		read(phdr[i].p_filesz);
	}
	/* Copy boot parameters back to original location */
	bcopy(saved_params, params, sizeof(struct bootparams));

	disable;
	_start();
	halt();
}
