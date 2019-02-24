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

#include <dev.h>
#include <dev/fd.h>
#include <errno.h>
#include <event.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys.h>
#include <sys/i8237.h>
#include <sys/i8259.h>
#include <sys/i8272.h>
#include <sys/intr.h>
#include <sys/ioctl.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/time.h>

#define SECTOR_SIZE		512

/* 
 * This device driver controls a 3 1/2", 1.44 Mbyte floppy disk that is
 * assumed to be in BIOS drive A:
 */
#define FD_DRIVE_A		0
#define FD_TRACKS		80
#define FD_HEADS		2
#define FD_SECTORS_PER_TRACK	18

#define FD_MODE_READ		0
#define FD_MODE_WRITE		1

#define FD_MTR_STAT_OFF		0
#define FD_MTR_STAT_DLY		1
#define FD_MTR_STAT_ON		2

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

/* 
 * XXX This is really ugly, but it works (TM).  The memory allocator knows
 * that the fd driver is using this page (fdbuf = 0x1000 assigned later on
 * in fd_init()) for its dma buffer and won't ever make use of it.
 */
static char *fdbuf;

static u_char bufp;
static u_char bufh;
static u_char bufl;
static u_char sizeh;
static u_char sizel;

static struct mutex fdmutex;
static int fdmotorstat = FD_MTR_STAT_OFF;
static int fdmotorcnt = 0;
static u_char fdhead = 0;
static u_char fdtrack = 0;
static u_char fdtrackprev = 0xff;
static u_char fdsector = 1;
static int fddone = 0;

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
    return (u_char) inb(I8272_DATA);
}

static void
fd_isr(void *params)
{
    /* Signal floppy operation completed */
    fddone = 1;

    /* Issue floppy eoi */
    outb(I8259_MSTR_CTRL, I8259_EOI_FD);
}

static void
fd_motorisr(void *params)
{
    if (fdmotorstat == FD_MTR_STAT_DLY) {
	fdmotorcnt -= tick;

	if (fdmotorcnt <= 0) {
	    fdmotorstat = FD_MTR_STAT_OFF;
	    outb(I8272_DOR, 0x0c);
	}
    }
}

static void
fd_wait()
{
    while (fddone == 0);
    fddone = 0;
}

int
fd_init(void *dev)
{
    u_int blks;

    fdbuf = (char *) 0x1000;
    bufp = (u_char) (((u_int) fdbuf) >> 16);
    bufh = (u_char) (((u_int) fdbuf) >> 8);
    bufl = (u_char) ((u_int) fdbuf);
    sizeh = (u_char) ((SECTOR_SIZE - 1) / 0x100);
    sizel = (u_char) (SECTOR_SIZE - 1);

    mutex_clear(&fdmutex);

    blks = FD_TRACKS * FD_HEADS * FD_SECTORS_PER_TRACK;
    kprintf("fd: %u blks %u trks %u hds %u sec/trk\n",
	    blks, FD_TRACKS, FD_HEADS, FD_SECTORS_PER_TRACK);

    isr_inst(INTR_TMR, fd_motorisr, NULL);
    isr_inst(INTR_FD, fd_isr, NULL);
    intr_unmask(INTR_FD);

    /* Set motor to known state */
    fd_ioctl(dev, MOTOR_ON, NULL);
    fd_ioctl(dev, MOTOR_OFF, NULL);

    return 0;
}

int
fd_ioctl(void *dev, int cmd, void *args)
{
    switch (cmd) {
    case LOCK:
	return mutex_lock(&fdmutex);

    case UNLOCK:
	return mutex_unlock(&fdmutex);

    case MOTOR_ON:
	if (fdmotorstat == FD_MTR_STAT_OFF) {
	    outb(I8272_DOR, (0x10 << (u_char) FD_DRIVE_A) | 0x0c);
	    delay(1);
	}
	fdmotorstat = FD_MTR_STAT_ON;
	return 0;

    case MOTOR_OFF:
	if (fdmotorstat == FD_MTR_STAT_ON) {
	    fdmotorstat = FD_MTR_STAT_DLY;
	    fdmotorcnt = 3000000;      /* 3 seconds */
	}
	return 0;

    case GET_GEOMETRY:
	{
	    geometry_t geom;

	    if (args == NULL)
		return EINVAL;
	    geom = (geometry_t) args;
	    geom->flags = GF_REMOVABLE;
	    geom->tracks = FD_TRACKS;
	    geom->heads = FD_HEADS;
	    geom->sectorspertrack = FD_SECTORS_PER_TRACK;
	    geom->bytespersector = SECTOR_SIZE;
	}
	return 0;

    case GET_BUFFER_SIZE:
	if (args == NULL)
	    return EINVAL;
	*((u_long *) args) = SECTOR_SIZE;
	return 0;

    case SEEK_BLOCK:
	{
	    seek_t seekargs;

	    if (args == NULL)
		return EINVAL;
	    seekargs = (seek_t) args;
	    if (seekargs->whence != SEEK_SET)
		return ENOSYS;

	    fdtrack = seekargs->offset / (FD_HEADS * FD_SECTORS_PER_TRACK);
	    fdhead = (seekargs->offset / FD_SECTORS_PER_TRACK) % FD_HEADS;
	    fdsector = seekargs->offset % FD_SECTORS_PER_TRACK + 1;
	}
	return 0;

    //default:
    }
    return ENOTTY;
}

static int
fd_transfer(u_char mode, char *buf)
{
    struct fd_result result;
    int retries = 3;

    if (buf == NULL)
	return EINVAL;

    if (mode == FD_MODE_WRITE)
	bcopy(buf, fdbuf, (size_t) SECTOR_SIZE);

    while (retries > 0) {
	/* Perform seek if necessary */
	if (fdtrack != fdtrackprev) {
	    fdtrackprev = fdtrack;

	    fd_command(I8272_SEEK);
	    fd_command((fdhead << 2) | FD_DRIVE_A);
	    fddone = 0;
	    fd_command(fdtrack);
	    fd_wait();
	    fd_command(I8272_SENSE);
	    result.st0 = fd_result();
	    result.track = fd_result();
	}
	/* Set up DMA */
	outb(I8237_DMA1_CHAN, 0x06);

	if (mode == FD_MODE_READ) {
	    outb(I8237_DMA1_RESET, I8237_DMA1_CHAN2_READ);
	    outb(I8237_DMA1_MODE, I8237_DMA1_CHAN2_READ);
	} else {
	    outb(I8237_DMA1_RESET, I8237_DMA1_CHAN2_WRITE);
	    outb(I8237_DMA1_MODE, I8237_DMA1_CHAN2_WRITE);
	}

	/* Setup DMA transfer */
	outb(I8237_DMA1_CHAN2_ADDR, bufl);
	outb(I8237_DMA1_CHAN2_ADDR, bufh);
	outb(I8237_DMA1_CHAN2_PAGE, bufp);
	outb(I8237_DMA1_CHAN2_COUNT, sizel);
	outb(I8237_DMA1_CHAN2_COUNT, sizeh);
	outb(I8237_DMA1_CHAN, 0x02);

	/* Perform transfer */
	if (mode == FD_MODE_READ) {
	    fd_command(I8272_READ);
	} else {
	    fd_command(I8272_WRITE);
	}
	fd_command((fdhead << 2) | FD_DRIVE_A);
	fd_command(fdtrack);
	fd_command(fdhead);
	fd_command(fdsector);
	fd_command(0x02);
	fd_command(0x12);
	fd_command(0x1b);
	fddone = 0;
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
	    /* 
	     * Abnormal command termination because the specified sector
	     * could not be found.  Recalibrate before retrying.
	     */
	    fdtrackprev = fdtrack;
	    fdtrack = 0;

	    fd_command(I8272_RECAL);
	    fddone = 0;
	    fd_command(0x00);
	    fd_wait();
	    fd_command(I8272_SENSE);
	    result.st0 = fd_result();
	    result.track = fd_result();

	} else if ((result.st0 & 0xc0) == 0) {

	    /* Successful transfer */
	    if (mode == FD_MODE_READ)
		bcopy(fdbuf, buf, (size_t) SECTOR_SIZE);
	    return 0;
	}
	retries--;
    }
    return ETIMEDOUT;
}

int
fd_read(void *dev, buf_t * b)
{
    int result;

    if (b == NULL || *b == NULL || bsize(*b) < SECTOR_SIZE)
	return EINVAL;

    fd_ioctl(dev, MOTOR_ON, NULL);
    if ((result = fd_transfer(FD_MODE_READ, bstart(*b))) == 0) {
	fd_ioctl(dev, MOTOR_OFF, NULL);
	blen(*b) = SECTOR_SIZE;
	return 0;
    }
    fd_ioctl(dev, MOTOR_OFF, NULL);
    blen(*b) = 0;
    return result;
}

int
fd_write(void *dev, buf_t * b)
{
    int result;

    if (b == NULL || *b == NULL)
	return EINVAL;

    fd_ioctl(dev, MOTOR_ON, NULL);
    result = fd_transfer(FD_MODE_WRITE, bstart(*b));
    fd_ioctl(dev, MOTOR_OFF, NULL);

    /* Discard buffer */
    brel(*b);
    *b = NULL;

    return result;
}

int
fd_shut(void *dev)
{
    return ENOSYS;
}
