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

#include <inttypes.h>
#include <stdio.h>
#include "rrfs.h"

void
printmbr(mbr_t mbr)
{
    printf("%u tracks %u heads %u sec/trk %u sectors\n",
	   (uint32_t) mbr->params.tracks,
	   (uint32_t) mbr->params.heads,
	   (uint32_t) mbr->params.sectorspertrack,
	   (uint32_t) mbr->params.sectors);
    printf("%u sec/fat %u sec/clust %u clusters\n",
	   (uint32_t) mbr->params.fatsectors,
	   (uint32_t) mbr->params.sectorsperclust,
	   (uint32_t) mbr->params.clusters);
}
