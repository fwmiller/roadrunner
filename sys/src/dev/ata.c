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
#include <dev/hd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys.h>
#include <sys/boot.h>
#include <sys/config.h>
#include <sys/i8259.h>
#include <sys/ioctl.h>

#define ATA_OUTB(ATAC, PORT, VAL)					\
{									\
    time_t start;							\
    u_char status;							\
									\
    for (start = time();;) {						\
	status = inb((ATAC)->iobase + ATA_ALT_STATUS);			\
	if (!(status & ATA_STAT_BSY) && !(status & ATA_STAT_DRQ)) {	\
	    outb((ATAC)->iobase + (PORT), (VAL));			\
	    break;							\
	}								\
	if (time() - start >= ATA_TIMEOUT_OUTB) {			\
	    ata_reset(ATAC);						\
	    return ETIMEDOUT;						\
	}								\
    }									\
}

#define ATA_WAIT(ATAC, CMD, MASK, TIMEOUT)				\
{									\
    time_t start;							\
    u_char status;							\
									\
    for (start = time();;) {						\
	status = inb((ATAC)->iobase + ATA_ALT_STATUS);			\
	if (!(status & ATA_STAT_BSY)) {					\
	    if (status & ATA_STAT_ERR)					\
		switch (CMD) {						\
		case ATA_CMD_READ:					\
		    return EDEVREAD;					\
		case ATA_CMD_WRITE:					\
		    return EDEVWRITE;					\
		default:						\
		    return ENOSYS;					\
		}							\
	    if ((status & (MASK)) == (MASK))				\
		break;							\
	}								\
	if (time() - start >= (TIMEOUT))				\
	    return ETIMEDOUT;						\
    }									\
}

static struct ata_controller atactab[ATA_CONTROLLERS];
static struct ata_drive atadtab[ATA_DRIVES];
static struct ata_partition ataptab[PARTS * ATA_DRIVES];
static int nextpart = 0;

static inline void ata_reset(atac_t atac)
{
	/* Assert software reset */
	outb(atac->iobase + ATA_CONTROL, ATA_CTL_RESET);

	/*
	 * XXX Should be 5 microseconds (ATA/ATAPI-6).  This would be better
	 * implemented with a Pentium Timestamp Counter based timer.
	 */
	DELAY(5000);

	/* Clear software reset */
	outb(atac->iobase + ATA_CONTROL, 0);
}

static void ata_convert_string(u_short * s, int words)
{
	char *s1;
	int i;

	for (i = 0; i < words; i++)
		s[i] = ((s[i] & 0xff) << 8) | ((s[i] >> 8) & 0xff);

	s1 = (char *)s;
	for (i = (words << 1) - 1; i >= 0; i--) {
		if (s1[i] != 0x20)
			break;
		s1[i] = '\0';
	}
}

static int ata_identify(atad_t atad, char *drvstr)
{
	/* Issue identify command */
	ATA_OUTB(atad->atac, ATA_DRVHD, 0xa0 | (atad->drive << 4));
	ATA_OUTB(atad->atac, ATA_COMMAND, ATA_CMD_IDENTIFY);

	/* Wait for data ready */
	ATA_WAIT(atad->atac, ATA_CMD_READ, ATA_STAT_DRQ, ATA_TIMEOUT_DRQ);

	/* Read parameter data */
	insw(atad->atac->iobase + ATA_DATA,
	     (void *)&(atad->param), SECTOR_SIZE / 2);

	/* Check for ATA device */
	if (atad->param.config & 0x8000) {
#if _DEBUG
		if ((atad->param.config & 0xc000) == 0x8000)
			kprintf("ata_identify: detected ATAPI device\n");
#endif
		return EFAIL;
	}
	/* Fill in drive parameters */
	atad->tracks = atad->param.cylinders;
	atad->heads = atad->param.heads;
	atad->sectorspertrack = atad->param.sectors;
	atad->blks = atad->tracks * atad->heads * atad->sectorspertrack;
	atad->size = (atad->blks * SECTOR_SIZE) / 1048576;

	ata_convert_string(atad->param.model, 20);
	kprintf("%s: ATA hard disk\n", drvstr);
	kprintf("%s: %s\n", drvstr, atad->param.model);
	kprintf("%s: %u blks (%d Mbytes) %u trks %u hds %u sec/trk\n",
		drvstr, atad->blks, atad->size,
		atad->tracks, atad->heads, atad->sectorspertrack);

	return 0;
}

static int atapi_identify(atad_t atad, char *drvstr)
{
	/* Issue identify packet command */
	ATA_OUTB(atad->atac, ATA_DRVHD, 0xa0 | (atad->drive << 4));
	ATA_OUTB(atad->atac, ATA_COMMAND, ATA_CMD_ATAPI_IDENTIFY);

	/* Wait for data ready */
	ATA_WAIT(atad->atac, ATA_CMD_READ, ATA_STAT_DRQ, ATA_TIMEOUT_DRQ);

	/* Read parameter data */
	insw(atad->atac->iobase + ATA_DATA,
	     (void *)&(atad->param), SECTOR_SIZE / 2);

	/* Check for ATAPI device */
	if ((atad->param.config & 0xc000) != 0x8000)
		return EFAIL;

	ata_convert_string(atad->param.model, 20);
	kprintf("%s: ATAPI ", drvstr);
	if (((atad->param.config >> 8) & 0x1f) == 5)
		kprintf("CD-ROM drive\n");
	else
		kprintf("device\n");
	kprintf("%s: %s\n", drvstr, atad->param.model);

	return 0;
}

static int ata_seek(atad_t atad, seek_t seekargs)
{
	if (seekargs->whence != SEEK_SET)
		return EINVAL;

	atad->blkno = seekargs->offset;
	atad->track = atad->blkno / (atad->heads * atad->sectorspertrack);
	atad->head = (atad->blkno / atad->sectorspertrack) % atad->heads;
	atad->sector = atad->blkno % atad->sectorspertrack + 1;
	return 0;
}

static int ata_read_mbr(atap_t atap, buf_t * b)
{
	struct seek seekargs;
	int result;

	seekargs.offset = 0;
	seekargs.whence = SEEK_SET;
	result = ata_seek(atap->atad, &seekargs);
	if (result < 0)
		return result;

	result = ata_read(atap, b);
	if (result < 0)
		return result;

	return 0;
}

static int ata_read_parttab(atad_t atad)
{
	buf_t b;
	struct ata_partition atap;
	int result;

	b = bget(SECTOR_SIZE);
	blen(b) = SECTOR_SIZE;
	atap.atad = atad;
	atap.sectors = 0;
	atap.offset = 0;
	result = ata_read_mbr(&atap, &b);
	if (result < 0) {
		brel(b);
		return result;
	}
	read_parttab((u_char *) bstart(b), atad->parttab);

	brel(b);
	return 0;
}

int ata_init()
{
	int drive, result;

	bzero(&atactab, ATA_CONTROLLERS * sizeof(struct ata_controller));
	bzero(&atadtab, ATA_DRIVES * sizeof(struct ata_drive));
	bzero(&ataptab, PARTS * ATA_DRIVES * sizeof(struct ata_partition));

	/* Initial configuration for both controllers */
	mutex_clear(&(atactab[0].mutex));
	atactab[0].iobase = ATA0_IOBASE;
	mutex_clear(&(atactab[1].mutex));
	atactab[1].iobase = ATA1_IOBASE;

	/* Reset both controllers */
	ata_reset(&(atactab[0]));
	ata_reset(&(atactab[1]));

	/* Scan for attached drives */
	for (drive = 0; drive < ATA_DRIVES; drive++) {
		atad_t atad = &(atadtab[drive]);
		char s[8];

		atad->atac = &(atactab[drive / 2]);
		atad->drive = drive % 2;

		/* Check for an ATA hard drive */
		sprintf(s, "ata%d", drive);
		result = ata_identify(atad, s);
		if (result == 0) {
			int part;

			atad->type = ATA_DRV_HD;
			result = ata_read_parttab(atad);
			if (result < 0)
				continue;

			for (part = 0; part < PARTS; part++) {
				atap_t atap;
				struct hd hd;

				atap = &(ataptab[nextpart++]);
				atap->atad = atad;
				atap->sectors = atad->parttab[part].size;
				atap->offset = atad->parttab[part].off;

				hd.ioctl = ata_ioctl;
				hd.read = ata_read;
				hd.write = ata_write;
				hd.part = atap;
				result = hd_alloc(&hd);
				if (result < 0)
					kprintf
					    ("ata_init: allocate hd failed (%s)\n",
					     strerror(result));
			}
			continue;
		}
		/* Check for an ATAPI device */
		result = atapi_identify(&(atadtab[drive]), s);
		if (result == 0)
			atadtab[drive].type = ATA_DRV_CDROM;
	}
	return 0;
}

int ata_get_boot_device(u_char drv, char *device)
{
	atad_t atad;
	int drive = 0, i, j;

	for (i = 0; i < ATA_DRIVES; i++) {
		atad = &(atadtab[i]);
		if (atad->type != ATA_DRV_NULL) {
			if (drive == (drv & 0x03)) {
				for (j = 0; j < PARTS; j++) {
					if (atad->parttab[j].active) {
						sprintf(device, "hd%d",
							(PARTS * drive) + j);
#if _DEBUG
						kprintf
						    ("ata_get_boot_device: ");
						kprintf
						    ("boot from partition %s\n",
						     device);
#endif
						return 0;
					}
				}
#if _DEBUG
				kprintf("ata_get_boot_device: ");
				kprintf("no active partition on ata%d\n", i);
#endif
				return ENXIO;
			}
			drive++;
		}
	}
#if _DEBUG
	kprintf("ata_get_boot_device: ");
	kprintf("illegal boot drive specified (%02x)\n", drv);
#endif
	return ENXIO;
}

int ata_ioctl(void *dev, int cmd, void *args)
{
	atap_t atap = (atap_t) dev;
	atad_t atad = atap->atad;

	switch (cmd) {
	case LOCK:
		return mutex_lock(&(atad->atac->mutex));

	case UNLOCK:
		return mutex_unlock(&(atad->atac->mutex));

	case GET_GEOMETRY:
		{
			geometry_t geom;

			if (args == NULL)
				return EINVAL;
			geom = (geometry_t) args;
			geom->flags = GF_PARTITIONED;
			geom->tracks = atad->param.cylinders;
			geom->heads = atad->param.heads;
			geom->sectorspertrack = atad->param.sectors;
			geom->bytespersector = SECTOR_SIZE;
		}
		return 0;

	case GET_BIOS_GEOMETRY:
		{
			geometry_t geom;

			if (args == NULL)
				return EINVAL;
			geom = (geometry_t) args;
			geom->flags = GF_PARTITIONED;
			geom->tracks =
			    (((bootparams.sec_cyl_hi & 0xc0) << 2) |
			     bootparams.cyl_lo) + 1;
			geom->heads = bootparams.hd + 1;
			geom->sectorspertrack = bootparams.sec_cyl_hi & 0x3f;
			geom->bytespersector = SECTOR_SIZE;
		}
		return 0;

	case READ_MBR:
		{
			buf_t b;
			int result;

			if (args == NULL)
				return EINVAL;

			b = bget(SECTOR_SIZE);
			blen(b) = SECTOR_SIZE;
			result = ata_read_mbr(atap, &b);
			if (result < 0) {
				brel(b);
				return result;
			}
			bcopy(bstart(b), (char *)args, SECTOR_SIZE);
			brel(b);
		}
		return 0;

	case WRITE_MBR:
		{
			buf_t b;
			struct seek seekargs;
			int result;

			if (args == NULL)
				return EINVAL;

			/*
			 * Update partition offsets.  This relies on the knowledge
			 * that the partitions for the specified drive were allocated
			 * in order.
			 */
			{
				struct part parttab[PARTS];
				int i, partcnt = 0;

				read_parttab((u_char *) args, parttab);
				for (i = 0; i < PARTS * ATA_DRIVES; i++)
					if (ataptab[i].atad == atad) {
						ataptab[i].sectors =
						    parttab[partcnt].size;
						ataptab[i].offset =
						    parttab[partcnt].off;
						if (++partcnt == PARTS)
							break;
					}
			}
			seekargs.whence = SEEK_SET;
			seekargs.offset = 0;
			result = ata_seek(atad, &seekargs);
			if (result < 0)
				return result;

			b = bget(SECTOR_SIZE);
			blen(b) = SECTOR_SIZE;
			bcopy((char *)args, bstart(b), SECTOR_SIZE);
			result = ata_write(atap, &b);
			if (result < 0) {
				if (b != NULL)
					brel(b);
				return result;
			}
		}
		return 0;

	case GET_PART_OFF:
		if (args == NULL)
			return EINVAL;
		*((u_long *) args) = atap->offset;
		return 0;

	case GET_PART_SIZE:
		if (args == NULL)
			return EINVAL;
		*((u_long *) args) = atap->sectors;
		return 0;

	case GET_BUFFER_SIZE:
		if (args == NULL)
			return EINVAL;
		*((u_long *) args) = SECTOR_SIZE;
		return 0;

	case SEEK_BLOCK:
		if (args == NULL)
			return EINVAL;
		((seek_t) args)->offset += atap->offset;
		return ata_seek(atad, (seek_t) args);

		//default:
	}
	return ENOTTY;
}

static inline void ata_eoi(atac_t atac)
{
	/* Clear drive interrupt */
	inb(atac->iobase + ATA_STATUS);

	/* Clear interrupt controllers */
	if (atac->iobase == ATA0_IOBASE)
		outb(I8259_SLV_CTRL, I8259_EOI_HD);
	else
		outb(I8259_SLV_CTRL, I8259_EOI_HD + 1);
	outb(I8259_MSTR_CTRL, I8259_EOI_CAS);
}

int ata_read(void *dev, buf_t * b)
{
	atap_t atap = (atap_t) dev;
	atad_t atad = atap->atad;
	u_short *buf;
	int i, nsectors;

	if (b == NULL || *b == NULL || blen(*b) % SECTOR_SIZE != 0)
		return EINVAL;

	nsectors = blen(*b) / SECTOR_SIZE;

	/* Select drive */
	ATA_OUTB(atad->atac, ATA_DRVHD,
		 0xa0 | ((u_char) atad->head & 0x0f) | (atad->drive << 4));

	/* Wait for drive ready */
	ATA_WAIT(atad->atac, ATA_CMD_READ, ATA_STAT_DRDY, ATA_TIMEOUT_DRDY);

	/* Issue read sectors command */
	ATA_OUTB(atad->atac, ATA_SECTORCNT, nsectors);
	ATA_OUTB(atad->atac, ATA_SECTOR, atad->sector);
	ATA_OUTB(atad->atac, ATA_TRACKLSB, atad->track);
	ATA_OUTB(atad->atac, ATA_TRACKMSB, (atad->track >> 8));
	ATA_OUTB(atad->atac, ATA_COMMAND, ATA_CMD_READ);

	for (i = 0; i < nsectors; i++) {
		/* Wait for data ready */
		ATA_WAIT(atad->atac, ATA_CMD_READ, ATA_STAT_DRQ,
			 ATA_TIMEOUT_DRQ);

		/* Read sector data */
		buf = (u_short *) (bstart(*b) + i * SECTOR_SIZE);
		insw(atad->atac->iobase + ATA_DATA, buf, SECTOR_SIZE / 2);

		ata_eoi(atad->atac);
	}
	return 0;
}

int ata_write(void *dev, buf_t * b)
{
	atap_t atap = (atap_t) dev;
	atad_t atad = atap->atad;
	u_short *buf;
	int i, j, nsectors;

	if (b == NULL || *b == NULL || blen(*b) % SECTOR_SIZE != 0)
		return EINVAL;

	nsectors = blen(*b) / SECTOR_SIZE;

	/* Select drive */
	ATA_OUTB(atad->atac, ATA_DRVHD,
		 0xa0 | ((u_char) atad->head & 0x0f) | (atad->drive << 4));

	/* Wait for drive ready */
	ATA_WAIT(atad->atac, ATA_CMD_WRITE, ATA_STAT_DRDY, ATA_TIMEOUT_DRDY);

	/* Issue write sectors command */
	ATA_OUTB(atad->atac, ATA_SECTORCNT, nsectors);
	ATA_OUTB(atad->atac, ATA_SECTOR, atad->sector);
	ATA_OUTB(atad->atac, ATA_TRACKLSB, atad->track);
	ATA_OUTB(atad->atac, ATA_TRACKMSB, (atad->track >> 8));
	ATA_OUTB(atad->atac, ATA_COMMAND, ATA_CMD_WRITE);

	for (i = 0; i < nsectors; i++) {
		/* Wait for data ready */
		ATA_WAIT(atad->atac, ATA_CMD_WRITE, ATA_STAT_DRQ,
			 ATA_TIMEOUT_DRQ);

		/* Write sector data */
		buf = (u_short *) (bstart(*b) + i * SECTOR_SIZE);
		for (j = 0; j < SECTOR_SIZE / 2; j++)
			outw(atad->atac->iobase + ATA_DATA, buf[j]);

		ata_eoi(atad->atac);
	}
	brel(*b);
	*b = NULL;

	return 0;
}
