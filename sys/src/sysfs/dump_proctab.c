/*
 *  Roadrunner/pk
 *    Copyright (C) 1989-2002  Cornfed Systems, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/proc.h>

#define PROC_HDR "pid  state  stdin  stdout  stderr  name"

static void
dump_proc(char **s, proc_t proc)
{
    sprintf(*s, "%3d      ", proc->slot);
    *s += 9;

    switch (proc->state) {
    case PS_READY:
	sprintf(*s, "r");
	break;
    case PS_RUN:
	sprintf(*s, "x");
	break;
    case PS_WAIT:
	sprintf(*s, "w");
	break;
    case PS_MUTEX:
	sprintf(*s, "m");
	break;
    case PS_EVENT:
	sprintf(*s, "e");
	break;
    case PS_SOCKET:
	sprintf(*s, "s");
	break;
    default:
	sprintf(*s, "E");
    }
    (*s)++;

    if (proc->fd[PFD_STDIN] < 0)
	sprintf(*s, "      -");
    else
	sprintf(*s, "  %5d", proc->fd[PFD_STDIN]);
    *s += 7;

    if (proc->fd[PFD_STDOUT] < 0)
	sprintf(*s, "       -");
    else
	sprintf(*s, "  %6d", proc->fd[PFD_STDOUT]);
    *s += 8;

    if (proc->fd[PFD_STDERR] < 0)
	sprintf(*s, "       -");
    else
	sprintf(*s, "  %6d", proc->fd[PFD_STDERR]);
    *s += 8;

    if (proc->context.argc > 0 && proc->context.argv != NULL) {
	int argvlen;

	/* Map the argv array of the process temporarily */
	argvlen = PAGE_SIZE + proc->context.argc * LINE_LENGTH;
	vm_map_range((pt_t) current->context.tss->cr3,
		     proc->context.argv, argvlen, PTE_PRESENT);

	if (proc->context.argv[0] != NULL) {
	    sprintf(*s, "  %s", proc->context.argv[0]);
	    *s += strlen(proc->context.argv[0]) + 2;
	}

	/* Remove mapping of argv array */
	vm_unmap_range((pt_t) current->context.tss->cr3,
		       proc->context.argv, argvlen);
    }
    sprintf(*s, "\n");
    (*s)++;
}

void
dump_proctab(char *s)
{
    proc_t proc;
    int i, proccnt;

    for (proccnt = 0, i = 0; i < PROCS; i++) {
	proc = &(proctab[i]);
	if (proc->state != PS_NULL) {
	    if (proccnt++ == 0) {
		sprintf(s, "%s\n", PROC_HDR);
		s += strlen(s);
	    }
	    dump_proc(&s, proc);
	}
    }
}
