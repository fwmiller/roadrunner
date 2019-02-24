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


#include <sys/i8254.h>
#include <sys/i8255.h>
#include <sys/i8259.h>
#include <sys/selector.h>


	.file		"asm.s"
	.text

	/*--------------------------------------------------------------------
	 *
	 * Switch to kernel data segments
	 *
	 */
	.align		4, 0x90
	.globl		kern_data_segs
kern_data_segs:
        pushl           %ebp
        movl            %esp, %ebp

	movl		$KDSEL, %ecx
	mov		%cx, %ds
	mov		%cx, %es
	mov		%cx, %fs
	mov		%cx, %gs

	leave
	ret

	/*--------------------------------------------------------------------
	 *
	 * Switch to user data segments
	 *
	 */
	.align		4, 0x90
	.globl		user_data_segs
user_data_segs:
        pushl           %ebp
        movl            %esp, %ebp

	movl		$UDSEL, %ecx
	mov		%cx, %ds
	mov		%cx, %es
	mov		%cx, %fs
	mov		%cx, %gs

	leave
	ret

	/*--------------------------------------------------------------------
	 *
	 * User level system call routine
	 *
	 * Two parameters are passed to the system call.  The first is the
	 * system call number as specified in <sys/syscall.h> and the second
	 * is a pointer to the first parameter of the calling routine.  The
	 * first and second parameters are passed to the kernel through the
	 * %eax and %ebx registers, respectively.
	 */
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

	/*--------------------------------------------------------------------
	 *
	 * Turn on paging
	 *
	 */
	.align		4, 0x90
	.globl		vm_enable
vm_enable:
        pushl           %ebp
        movl            %esp, %ebp

	movl		8(%ebp), %eax
        movl		%eax, %cr3
        movl		%cr0, %eax
        orl		$0x80000000, %eax
        movl		%eax, %cr0

        leave
        ret

	/*--------------------------------------------------------------------
	 *
	 * Get address causing a page fault
	 *
	 */
	.align		4, 0x90
	.globl		vm_pgfault_addr
vm_pgfault_addr:
        pushl           %ebp
        movl            %esp, %ebp

	movl		%cr2, %eax

        leave
        ret

	/*--------------------------------------------------------------------
	 *
	 * Perform context switch by jumping to Task State Segment (TSS)
	 *
	 */
	.align		4, 0x90
	.globl		jmptss
jmptss:
        pushl           %ebp
        movl            %esp, %ebp

        movl            8(%ebp), %edi
	sti

	/* Indirect inter-segment jump to a tss descriptor */
	.byte		0xff
	/* Mod r/m specifies ds:edi using mod=00 op=101 r/m=111 */
	.byte		0x2f

        leave
        ret

	/*--------------------------------------------------------------------
	 *
	 * Load the Task Register
	 *
	 */
	.align		4, 0x90
	.globl		ltr
ltr:
        pushl           %ebp
        movl            %esp, %ebp

        movl            8(%ebp), %eax
	ltr		%ax 

        leave
        ret

	/*--------------------------------------------------------------------
	 *
	 * System halt
	 *
	 */
	.align		4, 0x90
	.globl		halt
halt:
	hlt
	jmp		halt

	/*--------------------------------------------------------------------
	 *
	 * System restart
	 *
	 */
	.align		4, 0x90
	.globl		reboot
reboot:
	inb		$I8255_PORT_A, %al
	inb		$I8255_PORT_A, %al

	movb		$I8255_REBOOT, %al
	outb		%al, $I8255_CTRL

rebootloop:
	jmp		rebootloop

	/*--------------------------------------------------------------------
	 *
	 * Set instruction pointer for new process
	 *
	 */
	.align		4, 0x90
	.globl		vfork_return
vfork_return:
        pushl           %ebp
        movl            %esp, %ebp

	jmp		return_setup

new_process:
	xorl		%eax, %eax

        leave
        ret

return_setup:
	movl		4(%ebp), %edi
	movl		new_process, %eax
	movl		%eax, (%edi)
	movl		8(%ebp), %eax

        leave
        ret

	/*--------------------------------------------------------------------
	 *
	 * Read Pentium timestamp counter
	 *
	 */
	.globl		tscread
tscread:
	pushl		%ebp
	movl		%esp, %ebp

	movl		8(%ebp), %edi

	/* Opcode for reading Pentium timestamp counter */
	.byte		0x0f
	.byte		0x31

	movl		%edx, (%edi)
	addl		$4, %edi
	movl		%eax, (%edi)

	leave
	ret
	.align		2, 0x90
