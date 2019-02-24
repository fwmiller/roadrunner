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


#define D32		.byte 0x66;
#define A32		.byte 0x67;

	.file		"boot2.s"
	.text

boot2:
	/* Set data segment */
	D32 xorl	%eax, %eax
	mov		%ax, %ds

	/* Disable interrupts */
        cli

	/* Load gdtr */
	A32 D32 lgdt	gdtptr
	A32 D32 lidt	idtptr

	/* Switch to protected mode */
	movl		%cr0, %eax
	D32 orl		$1, %eax
	movl		%eax, %cr0

        /* Set code selector */
	D32 ljmp	$0x08, $setcs
setcs:
	/* Set segment selectors */
	movl		$0x10, %eax
	mov		%ax, %ds
	mov		%ax, %es
	mov		%ax, %fs
	mov		%ax, %gs
	mov		%ax, %ss

	/* Set initial stack ptr just below the video buffer */
	D32 movl	$0x0009fff8, %esp

	call		boot
	.align		4, 0x90

fdisr:
	cli
	call		fd_isr
	sti
	iret
	.align		2, 0x90

	.data

	/* Global descriptor table for boot2 */
gdt:
	/* Null descriptor */
	.long		0
	.long		0

	/* Code descriptor */
	.short		0xffff
	.short		0
	.byte		0
	.byte		0x9f
	.byte		0xcf
	.byte		0

	/* Data descriptor */
	.short		0xffff
	.short		0
	.byte		0
	/* .byte		0x93 */
	.byte		0x92
	.byte		0xcf
	.byte		0
	.align          2, 0x90

#define INTR_DESC(INTR)			\
	.short		INTR;		\
	.short		0x08;		\
	.byte		0;		\
	.byte		0x8f;		\
	.short		0

	/* Interrupt descriptor table for boot2 */
idt:
	.space		38 * 8		/* Interrupts 0..37 not used */
	INTR_DESC(fdisr)		/* Floppy disk interrupt 38 */
	.space		9 * 8		/* Interrupts 39..47 not used */

	/* Global descriptor table ptr for boot2 */
gdtptr:
	.short		(3 * 8) - 1
	.long		gdt

	/* Interrupt descriptor table ptr for boot2 */
idtptr:
	.short		(48 * 8) - 1
	.long		idt
