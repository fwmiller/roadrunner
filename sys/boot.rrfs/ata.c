#include <dev/ata.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys.h>
#include <sys/config.h>
#include <sys/i8259.h>

#define WAIT_COUNT	100000

static atac_t atactab;
static atad_t atadtab;
static atap_t ataptab;
static int nextpart = 0;

atap_t atap_boot = NULL;

static void ata_outb(atac_t atac, u_short port, u_char val)
{
	u_char status;
	int i;

	for (i = WAIT_COUNT; i > 0; i--) {
		status = inb(atac->iobase + ATA_ALT_STATUS);
		if (!(status & ATA_STAT_BSY) && !(status & ATA_STAT_DRQ)) {
			outb(atac->iobase + port, val);
			break;
		}
	}
}

static void ata_wait(atac_t atac, u_char mask)
{
	u_char status;
	int i;

	for (i = WAIT_COUNT; i > 0; i--) {
		status = inb(atac->iobase + ATA_ALT_STATUS);
		if (!(status & ATA_STAT_BSY)) {
			if (status & ATA_STAT_ERR)
				/* XXX Probably want an error message and halt */
				continue;
			if ((status & mask) == mask)
				break;
		}
	}
}

#if _DEBUG
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
#endif

static int ata_identify(atad_t atad, char *drvstr)
{
	/* Issue identify command */
	ata_outb(atad->atac, ATA_DRVHD, 0xa0 | (atad->drive << 4));
	ata_outb(atad->atac, ATA_COMMAND, ATA_CMD_IDENTIFY);

	/* Wait for data ready */
	ata_wait(atad->atac, ATA_STAT_DRQ);

	/* Read parameter data */
	insw(atad->atac->iobase + ATA_DATA,
	     (void *)&(atad->param), SECTOR_SIZE / 2);

	/* Check for ATA device */
	if (atad->param.config & 0x8000)
		return EFAIL;

	/* Fill in drive parameters */
	atad->tracks = atad->param.cylinders;
	atad->heads = atad->param.heads;
	atad->sectorspertrack = atad->param.sectors;
	atad->blks = atad->tracks * atad->heads * atad->sectorspertrack;
	atad->size = (atad->blks * SECTOR_SIZE) / 1048576;
#if _DEBUG
	ata_convert_string(atad->param.model, 20);
	printf("%s: ATA hard disk\n", drvstr);
	printf("%s: %s\n", drvstr, atad->param.model);
	printf("%s: %u blks (%d Mbytes) %u trks %u hds %u sec/trk\n",
	       drvstr, atad->blks, atad->size,
	       atad->tracks, atad->heads, atad->sectorspertrack);
#endif
	return 0;
}

void read_parttab(char *mbr, part_t parttab)
{
	part_t p;
	u_int partent;
	char *soff, *eoff;
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

#if _DEBUG
void dump_parttab(part_t parttab)
{
	int i;

	printf("part  type\n");
	for (i = 0; i < PARTS; i++) {
		printf("%d     ", i);
		if (parttab[i].sys == PART_SYS_RRFS)
			printf("rrfs\n");
		else
			printf("-\n");
	}
}
#endif

void ata_readblk(atad_t atad, u_long blkno, char *b);

static void ata_read_parttab(atad_t atad)
{
	char b[SECTOR_SIZE];

	ata_readblk(atad, 0, b);
	read_parttab(b, atad->parttab);
#if _DEBUG
	dump_parttab(atad->parttab);
#endif
}

static int atapi_identify(atad_t atad, char *drvstr)
{
	/* Issue identify packet command */
	ata_outb(atad->atac, ATA_DRVHD, 0xa0 | (atad->drive << 4));
	ata_outb(atad->atac, ATA_COMMAND, ATA_CMD_ATAPI_IDENTIFY);

	/* Wait for data ready */
	ata_wait(atad->atac, ATA_STAT_DRQ);

	/* Read parameter data */
	insw(atad->atac->iobase + ATA_DATA,
	     (void *)&(atad->param), SECTOR_SIZE / 2);

	/* Check for ATAPI device */
	if ((atad->param.config & 0xc000) != 0x8000)
		return EFAIL;
#if _DEBUG
	ata_convert_string(atad->param.model, 20);
	printf("%s: ATAPI ", drvstr);
	if (((atad->param.config >> 8) & 0x1f) == 5)
		printf("CD-ROM drive\n");
	else
		printf("device\n");
	printf("%s: %s\n", drvstr, atad->param.model);
#endif
	return 0;
}

void ata_set_boot_device(u_char drv)
{
	atad_t atad;
	int drive = 0, i, j, partno = 0;

	for (i = 0; i < ATA_DRIVES; i++) {
		atad = &(atadtab[i]);
		if (atad->type != ATA_DRV_NULL) {
			if (drive == (drv & 0x03)) {
				for (j = 0; j < PARTS; j++) {
					if (atad->parttab[j].active) {
						atap_boot =
						    &(ataptab[partno + j]);
						return;
					}
				}
#if _DEBUG
				printf("ata_set_boot_device: ");
#endif
				printf("no active partition on ata%d\n", i);
				halt();
			}
			if (atad->type == ATA_DRV_HD)
				partno += PARTS;
			drive++;
		}
	}
#if _DEBUG
	printf("ata_set_boot_device: ");
#endif
	printf("illegal boot drive specified\n");
	halt();
}

int ata_init()
{
	int drive, result;

	atactab =
	    (atac_t) malloc(ATA_CONTROLLERS * sizeof(struct ata_controller));
	atadtab = (atad_t) malloc(ATA_DRIVES * sizeof(struct ata_drive));
	ataptab =
	    (atap_t) malloc(PARTS * ATA_DRIVES * sizeof(struct ata_partition));
	bzero(atactab, ATA_CONTROLLERS * sizeof(struct ata_controller));
	bzero(atadtab, ATA_DRIVES * sizeof(struct ata_drive));
	bzero(ataptab, PARTS * ATA_DRIVES * sizeof(struct ata_partition));

	atactab[0].iobase = ATA0_IOBASE;
	atactab[1].iobase = ATA1_IOBASE;

	for (drive = 0; drive < ATA_DRIVES; drive++) {
		atad_t atad = &(atadtab[drive]);
		char s[8];

		atad->atac = &(atactab[drive / 2]);
		atad->drive = drive % 2;

		if (drive == 0)
			strcpy(s, "ata0");
		else if (drive == 1)
			strcpy(s, "ata1");
		else if (drive == 2)
			strcpy(s, "ata2");
		else if (drive == 3)
			strcpy(s, "ata3");

		result = ata_identify(atad, s);
		if (result == 0) {
			int part;

			atad->type = ATA_DRV_HD;
			ata_read_parttab(atad);

			for (part = 0; part < PARTS; part++) {
				atap_t atap = &(ataptab[nextpart++]);

				atap->atad = atad;
				atap->sectors = atad->parttab[part].size;
				atap->offset = atad->parttab[part].off;
			}
			continue;
		}
		result = atapi_identify(&(atadtab[drive]), s);
		if (result == 0)
			atadtab[drive].type = ATA_DRV_CDROM;
	}
	return 0;
}

void ata_seek(atad_t atad, u_long offset)
{
	atad->blkno = offset;
	atad->track = atad->blkno / (atad->heads * atad->sectorspertrack);
	atad->head = (atad->blkno / atad->sectorspertrack) % atad->heads;
	atad->sector = atad->blkno % atad->sectorspertrack + 1;
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

void ata_readblk(atad_t atad, u_long blkno, char *b)
{
	u_short *wdbuf;

	ata_seek(atad, blkno);
#if _DEBUG
	printf("ata_readblk: blkno %u trk %u hd %u sec %u\n",
	       atad->blkno, atad->track, atad->head, atad->sector);
#endif
	/* Select drive */
	ata_outb(atad->atac, ATA_DRVHD,
		 0xa0 | ((u_char) atad->head & 0x0f) | (atad->drive << 4));

	/* Wait for drive ready */
	ata_wait(atad->atac, ATA_STAT_DRDY);

	/* Issue read sectors command */
	ata_outb(atad->atac, ATA_SECTORCNT, 1);
	ata_outb(atad->atac, ATA_SECTOR, atad->sector);
	ata_outb(atad->atac, ATA_TRACKLSB, atad->track);
	ata_outb(atad->atac, ATA_TRACKMSB, (atad->track >> 8));
	ata_outb(atad->atac, ATA_COMMAND, ATA_CMD_READ);

	/* Wait for data ready */
	ata_wait(atad->atac, ATA_STAT_DRQ);

	/* Read sector data */
	wdbuf = (u_short *) b;
	insw(atad->atac->iobase + ATA_DATA, wdbuf, SECTOR_SIZE / 2);

	ata_eoi(atad->atac);
}
