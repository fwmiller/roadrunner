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


	.file		"_syscall.s"
	.text

/*****************************************************************************
*                                                                            *
* User level system call routine                                             *
*                                                                            *
* Two parameters are passed to the system call.  The first is the system     *
* call number as specified in <sys/syscall.h> and the second is a pointer    *
* to the first parameter of the calling routine.  The first and second       *
* parameters are passed to the kernel through the %eax and %ebx registers,   *
* respectively.                                                              *
*                                                                            *
*****************************************************************************/

	.align		4, 0x90
	.globl		syscall
syscall:
        pushl           %ebp
        movl            %esp, %ebp

	movl		8(%ebp), %eax	/* System call number */
	movl		12(%ebp), %ebx	/* Pointer to first parameter */
	int		$48

	leave
	ret
