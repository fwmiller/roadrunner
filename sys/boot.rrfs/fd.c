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

#include <string.h>
#include <sys.h>
#include <sys/i8237.h>
#include <sys/i8259.h>
#include <sys/i8272.h>
#include <sys/types.h>

#define SECTOR_SIZE		512

/* 
 * This device driver controls a 3 1/2", 1.44 Mbyte floppy disk that is
 * assumed to be in drive a:
 */
#define FD_DRIVE_A		0
#define FD_TRACKS		80
#define FD_HEADS		2
#define FD_SECTORS_PER_TRACK	18

#define FD_MODE_READ		0

struct fd_result {
    u_char st0;
    u_char st1;
    u_char st2;
    u_char st3;
    u_char track;
    u_char head;
    u_char sector;
    u_char size;
};

struct fd {
    char *buf;
    u_char bufp;
    u_char bufh;
    u_char bufl;
    u_char sizeh;
    u_char sizel;
    u_char head;
    u_char prevtrack;
    u_char track;
    u_char sector;
    int done;
};

static struct fd fd0;

void
fd_init()
{
    fd0.buf = (char *) 0x1000;
    fd0.bufp = (u_char) (((u_int) fd0.buf) >> 16);
    fd0.bufh = (u_char) (((u_int) fd0.buf) >> 8);
    fd0.bufl = (u_char) ((u_int) fd0.buf);
    fd0.sizeh = (u_char) ((SECTOR_SIZE - 1) / 0x100);
    fd0.sizel = (u_char) (SECTOR_SIZE - 1);
    fd0.head = 0xff;
    fd0.prevtrack = 0;
    fd0.track = 0xff;
    fd0.sector = 0xff;
    fd0.done = 0;
}

static void
fd_command(u_char c)
{
    u_char status;

    do
	status = inb(I8272_STATUS);
    while ((status & 0x40) || !(status & 0x80));
    outb(I8272_DATA, c);
}

static u_char
fd_result()
{
    u_char status;

    do
	status = inb(I8272_STATUS);
    while (!(status & 0x40) || !(status & 0x80));
    return inb(I8272_DATA);
}

void
fd_isr()
{
    /* Issue floppy eoi */
    outb(I8259_MSTR_CTRL, I8259_EOI_FD);

    /* Signal floppy operation completed */
    fd0.done = 1;
}

static void
fd_seek(u_int blkno)
{
    fd0.track = blkno / (FD_HEADS * FD_SECTORS_PER_TRACK);
    fd0.head = (blkno / FD_SECTORS_PER_TRACK) % FD_HEADS;
    fd0.sector = blkno % FD_SECTORS_PER_TRACK + 1;
}

static inline void
fd_wait()
{
    while (fd0.done == 0);
    fd0.done = 0;
}

static int
fd_readsector(char *buf)
{
    struct fd_result result;
    int retries = 3;

    while (retries > 0) {
	/* Perform seek if necessary */
	if (fd0.track != fd0.prevtrack) {
	    fd_command(I8272_SEEK);
	    fd_command((fd0.head << 2) | FD_DRIVE_A);
	    fd0.done = 0;
	    fd_command(fd0.track);
	    fd_wait();
	    fd_command(I8272_SENSE);
	    result.st0 = fd_result();
	    result.track = fd_result();
	}
	/* Set up DMA */
	outb(I8237_DMA1_CHAN, 0x06);
	outb(I8237_DMA1_RESET, I8237_DMA1_CHAN2_READ);
	outb(I8237_DMA1_MODE, I8237_DMA1_CHAN2_READ);

	/* Setup DMA transfer */
	outb(I8237_DMA1_CHAN2_ADDR, fd0.bufl);
	outb(I8237_DMA1_CHAN2_ADDR, fd0.bufh);
	outb(I8237_DMA1_CHAN2_PAGE, fd0.bufp);
	outb(I8237_DMA1_CHAN2_COUNT, fd0.sizel);
	outb(I8237_DMA1_CHAN2_COUNT, fd0.sizeh);
	outb(I8237_DMA1_CHAN, 0x02);

	/* Perform transfer */
	fd_command(I8272_READ);
	fd_command((fd0.head << 2) | FD_DRIVE_A);
	fd_command(fd0.track);
	fd_command(fd0.head);
	fd_command(fd0.sector);
	fd_command(0x02);
	fd_command(0x12);
	fd_command(0x1b);
	fd0.done = 0;
	fd_command(0xff);
	fd_wait();
	result.st0 = fd_result();
	result.st1 = fd_result();
	result.st2 = fd_result();
	result.track = fd_result();
	result.head = fd_result();
	result.sector = fd_result();
	result.size = fd_result();

	if ((result.st0 & 0xc0) == 0x40 && (result.st1 & 0x04) == 0x04) {
	    /* Recalibrate before retrying */
	    fd0.prevtrack = fd0.track;
	    fd0.track = 0;
	    fd_command(I8272_RECAL);
	    fd0.done = 0;
	    fd_command(0x00);
	    fd_wait();
	    fd_command(I8272_SENSE);
	    result.st0 = fd_result();
	    result.track = fd_result();
	} else if ((result.st0 & 0xc0) == 0) {
	    /* Successful transfer */
	    bcopy(fd0.buf, buf, SECTOR_SIZE);
	    return 0;
	}
	retries--;
    }
    return (-1);
}

int
fd_read(u_int blkno, char *b)
{
    /* Assume motor is on */
    fd_seek(blkno);
    return fd_readsector(b);
}
