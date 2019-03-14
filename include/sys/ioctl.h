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

#ifndef __IOCTL_H
#define __IOCTL_H

#include <sys/types.h>

/* Global ioctl commands and arguments */
#define LOCK			0xffff
#define UNLOCK			0xfffe
#define MOTOR_ON		0xfffd
#define MOTOR_OFF		0xfffc
#define GET_GEOMETRY		0xfffb
#define GET_BIOS_GEOMETRY	0xfffa
#define READ_MBR		0xfff9
#define WRITE_MBR		0xfff8
#define GET_PART_OFF		0xfff7
#define GET_PART_SIZE		0xfff6
#define GET_BUFFER_SIZE		0xfff5
#define GET_FILE_SIZE		0xfff4
#define GET_FILE_POS		0xfff3
#define SEEK_BYTE		0xfff2
#define SEEK_BLOCK		0xfff1
#define GET_WINDOW_HEIGHT	0xfff0
#define GET_WINDOW_WIDTH	0xffef
#define GET_CURSOR_POS		0xffee
#define GET_CURSOR_CHAR		0xffed
#define SET_CURSOR_ON		0xffec
#define SET_CURSOR_OFF		0xffeb
#define SET_CURSOR_POS		0xffea
#define SET_NORMAL_VIDEO	0xffe9
#define SET_BRIGHT_VIDEO	0xffe8
#define SET_REVERSE_VIDEO	0xffe7
#define SET_BLINK_VIDEO		0xffe6
#define CLEAR			0xffe5
#define CLEAR_TO_EOL		0xffe4

/* GET_GEOMETRY */
typedef struct geometry
{
#define GF_REMOVABLE		0x01
#define GF_PARTITIONED		0x02
  int flags;
  u_long tracks;
  u_long heads;
  u_long sectorspertrack;
  u_short bytespersector;
} *geometry_t;

/* SEEK_BYTE and SEEK_BLOCK */
typedef struct seek
{
  u_long offset;
#define SEEK_SET		0
#define SEEK_CUR		1
#define SEEK_END		2
  int whence;
} *seek_t;

/* GET_CURSOR_POS SET_CURSOR_POS */
typedef struct cursor
{
  int x, y;
} *cursor_t;

int ioctl (int fd, int cmd, void *args);

#endif
