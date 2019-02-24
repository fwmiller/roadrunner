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

#ifndef __TMR_H
#define __TMR_H

#if _KERNEL

#include <sys/types.h>

/* Routines for managing the i8254 counter 0 periodic timer */

/* Compute a count value for a specified frequency */
u_int tmrcount(u_int freq);

/* Get the number of microseconds per counter 0 expiration */
u_int tmrtick();

/* Start the i8254 counter 0 periodic timer */
void tmrstart(u_int count);

/* Read the current value of counter 0 */
u_int tmrread();

#endif				/* _KERNEL */

#endif
