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

#include <errno.h>
#include <stdio.h>
#include <sys/ioctl.h>

int
feof(FILE * stream)
{
    u_long pos = 0, size = 0;

    /*
     * Get and test the attributes associated with the stream necessary
     * to determine whether the end-of-file has been reached
     */
    ioctl((int) stream, GET_FILE_POS, &pos);
    ioctl((int) stream, GET_FILE_SIZE, &size);
    if (size - pos > 0)
	return 0;
    return 1;
}
