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

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys.h>
#include <sys/config.h>
#include <sys/ioctl.h>
#include <sys/part.h>
#include <unistd.h>

static void
getline(char *s, int len)
{
    int ch, i;

    for (i = 0; i < len - 1;) {
	ch = getchar();
	if (isprint(ch))
	    printf("%c", ch);

	if (ch == 0 || ch == '\n')
	    break;
	if (ch == '\b' && i > 0) {
	    s[--i] = '\0';
	    printf("\b");
	    continue;
	} else if (ch == '\b')
	    continue;

	s[i++] = (char) ch;
    }
    printf("\n");
}

static int
getpartno()
{
    int ch, partno;

    printf("enter partition number (0-%d)? ", PARTS - 1);
    ch = getchar();
    if (isprint(ch))
	printf("%c", ch);
    printf("\n");

    partno = ch - '0';
    if (partno < 0 || partno >= PARTS) {
	printf("illegal partition number\n");
	return (-1);
    }
    return partno;
}

static void
getdiskcoord(geometry_t geom, int blkno, u_int * track, u_int * head,
	     u_int * sector)
{
    *track = blkno / (geom->heads * geom->sectorspertrack);
    *head = (blkno / geom->sectorspertrack) % geom->heads;
    *sector = blkno % geom->sectorspertrack + 1;
}

static void
addpart(part_t parttab, geometry_t geom)
{
    char line[80];
    int off, noff, poff;
    int size, psize;
    int blks, i, partno;

    partno = getpartno();
    if (partno < 0)
	return;
    if (parttab[partno].sys != PART_SYS_NONE) {
	printf("partition in use\n");
	return;
    }
    /* Get user specified partition size */
    printf("partition size (in Kbytes)? ");
    getline(line, 80);
    size = (atoi(line) * 1024) / geom->bytespersector;

    /* Adjust size to be a multiple of the sectors per track */
    size = ALIGN(size, geom->sectorspertrack);
    if (size < geom->sectorspertrack) {
	printf("partition size must be at least %d sectors\n",
	       (int) geom->sectorspertrack);
	return;
    }
    /* Search for the next partition in the table */
    for (i = partno + 1; i < PARTS; i++)
	if (parttab[i].sys != PART_SYS_NONE)
	    break;
    if (i >= PARTS) {
	printf("no next partition\n");
	noff = (-1);
    } else {
	printf("next partition %d\n", i);
	noff = parttab[i].off;
    }

    /* Search for the previous partition in the table */
    for (i = partno - 1; i >= 0; i--)
	if (parttab[i].sys != PART_SYS_NONE)
	    break;
    if (i < 0) {
	printf("no previous partition\n");
	poff = (-1);
    } else {
	printf("previous partition %d\n", i);
	poff = parttab[i].off;
	psize = parttab[i].size;
    }
    /* Compute total number of sectors on the disk */
    blks = geom->tracks * geom->heads * geom->sectorspertrack;

    /* Compute offset of new partition */
    if (poff < 0)
	off = geom->sectorspertrack;
    else
	off = ALIGN(poff + psize, geom->sectorspertrack);

    /* Check whether specified partition will fit */
    if (off + size >= (noff < 0 ? blks : noff)) {
	printf("partition size too large\n");
	return;
    }
    printf("add partition %d offset %d size %d\n", partno, off, size);

    parttab[partno].off = off;
    parttab[partno].size = size;
    parttab[partno].sys = PART_SYS_RRFS;

    getdiskcoord(geom, off, &(parttab[partno].st),
		 &(parttab[partno].sh), &(parttab[partno].ss));
    getdiskcoord(geom, off + size - 1, &(parttab[partno].et),
		 &(parttab[partno].eh), &(parttab[partno].es));

    /* Dump partition table */
    printf("\n");
    dump_parttab(parttab);
}

static void
bootpart(part_t parttab)
{
    int partno, i;

    partno = getpartno();
    if (partno < 0)
	return;
    for (i = 0; i < PARTS; i++)
	parttab[i].active = 0;
    parttab[partno].active = 0x80;

    /* Dump partition table */
    printf("\n");
    dump_parttab(parttab);
}

static void
delpart(part_t parttab)
{
    int ch, partno;

    partno = getpartno();
    if (partno < 0)
	return;
    if (parttab[partno].sys == PART_SYS_NONE) {
	printf("partition not in use\n");
	return;
    }
    printf("delete partition %d\n", partno);

    printf("\nARE YOU SURE (uppercase 'Y' to confirm)? ");
    ch = getchar();
    if (isprint(ch))
	printf("%c", ch);
    printf("\n");

    if (ch == 'Y') {
	bzero(&(parttab[partno]), sizeof(struct part));

	/* Dump partition table */
	printf("\n");
	dump_parttab(parttab);
    }
}

static int
commit(part_t parttab, int devno)
{
    int ch, result;

    printf("write partition table\n");
    printf("\nARE YOU SURE (uppercase 'Y' to confirm)? ");
    ch = getchar();
    if (isprint(ch))
	printf("%c", ch);
    printf("\n");

    if (ch == 'Y') {
	char mbr[SECTOR_SIZE];

	result = ioctl(devno, READ_MBR, mbr);
	if (result < 0) {
	    printf("read mbr failed (%s)\n", strerror(result));
	    return result;
	}
	write_parttab(parttab, mbr);

	result = ioctl(devno, WRITE_MBR, mbr);
	if (result < 0) {
	    printf("write mbr failed (%s)\n", strerror(result));
	    return result;
	}
    }
    return 0;
}

static void
parttype(part_t parttab)
{
    char line[80];
    int len, partno, sys = 0;

    partno = getpartno();
    if (partno < 0)
	return;

    /* Get user specified partition type */
    printf("partition type (0 <= hex value <= ff)? ");
    getline(line, 80);

    len = strlen(line);
    if (len < 1 || len > 2)
	goto parttype_error;

    if (!isxdigit(line[0]))
	goto parttype_error;

    if (line[0] >= '0' && line[0] <= '9') {
	if (len > 1)
	    sys += (line[0] - '0') << 4;
	else
	    sys += line[0] - '0';
    } else if (line[0] >= 'a' && line[0] <= 'f') {
	if (len > 1)
	    sys += (line[0] - 'a' + 10) << 4;
	else
	    sys += line[0] - 'a' + 10;
    } else {
	if (len > 1)
	    sys += (line[0] - 'A' + 10) << 4;
	else
	    sys += line[0] - 'A' + 10;
    }
    if (len > 1 && !isxdigit(line[1]))
	goto parttype_error;

    if (len > 1) {
	if (line[1] >= '0' && line[1] <= '9')
	    sys += line[1] - '0';
	else if (line[1] >= 'a' && line[1] <= 'f')
	    sys += line[1] - 'a' + 10;
	else
	    sys += line[1] - 'A' + 10;
    }
    parttab[partno].sys = sys;
    return;

  parttype_error:
    printf("illegal partition type value\n");
}

int
main(int argc, char **argv)
{
    struct geometry geom;
    struct part parttab[PARTS];
    int ch, devno, done, result;

    if (argc < 2) {
	printf("missing argument\n");
	return EINVAL;
    }
    if (argc > 2) {
	printf("too many arguments\n");
	return EINVAL;
    }
    /* Open disk device */
    result = open(argv[1], O_RDWR);
    if (result < 0) {
	printf("could not open %s (%s)\n", argv[1], strerror(result));
	return result;
    }
    devno = result;

    /* Lock disk device */
    result = ioctl(devno, LOCK, NULL);
    if (result < 0) {
	printf("lock failed (%s)\n", strerror(result));
	close(devno);
	return result;
    }
    /* Get disk geometry */
    result = ioctl(devno, GET_BIOS_GEOMETRY, &geom);
    if (result < 0) {
	printf("get geometry failed (%s)\n", strerror(result));
	ioctl(devno, UNLOCK, NULL);
	close(devno);
	return result;
    }
#if _DEBUG
    printf("fdisk: bios geometry %u trks %u hds %u sec/trk\n",
	   (u_int) geom.tracks,
	   (u_int) geom.heads, (u_int) geom.sectorspertrack);
#endif

    /* Read partition table */
    {
	char mbr[SECTOR_SIZE];

	result = ioctl(devno, READ_MBR, mbr);
	if (result < 0) {
	    printf("read mbr failed (%s)\n", strerror(result));
	    ioctl(devno, UNLOCK, NULL);
	    close(devno);
	    return result;
	}
	read_parttab(mbr, parttab);
    }
    dump_parttab(parttab);

    /* User commands */
    for (done = 0; !done;) {
	printf("(a)dd (b)oot (t)ype (c)ommit (d)elete (p)rint e(x)it\n");
	printf("fdisk> ");
	ch = getchar();
	if (isprint(ch))
	    printf("%c", ch);
	printf("\n");

	if (ch == 'a')
	    addpart(parttab, &geom);
	else if (ch == 'b')
	    bootpart(parttab);
	else if (ch == 'c')
	    commit(parttab, devno);
	else if (ch == 'd')
	    delpart(parttab);
	else if (ch == 'p') {
	    printf("\n");
	    dump_parttab(parttab);
	} else if (ch == 't')
	    parttype(parttab);
	else if (ch == 'x')
	    done = 1;
	else if (isprint(ch))
	    printf("command not recognized\n");
    }
    ioctl(devno, UNLOCK, NULL);
    close(devno);
    return 0;
}
