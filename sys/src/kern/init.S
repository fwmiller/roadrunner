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


#include <sys/config.h>
#include <sys/selector.h>


	.file		"init.s"
	.text

	/* Assume we are already running in protected mode */
	.align		4, 0x90
	.globl		_start
_start:
	cli

	/* Clear .bss */
	xorl		%eax, %eax
	movl		$_edata, %edi
	movl		$_end, %ecx
	subl		%edi, %ecx
	cld
	rep
	stosb

	/* Setup global descriptor table */
	call		gdtinit

	/* Load global descriptor table register */
	.extern		gdtptr
	lgdt		gdtptr

	/* Set code segment and clear prefetch */
	ljmp		$KCSEL, $setcs
setcs:

	/* Load segment registers */
	movl		$KDSEL, %eax
	mov		%ax, %ds
	mov		%ax, %ss
	mov		%ax, %es
	mov		%ax, %fs
	mov		%ax, %gs

	/* Load initial stack pointer */
	movl		$initkstk, %esp

	/* Load interrupt descriptor table register */
	lidt		idtptr

	/* Load idt */
	lea		idt, %edi

#define LOAD_INTR_DESC(INTR)						\
	lea		INTR, %edx;					\
	movl		$(KCSEL << 16), %eax;				\
	movw		%dx, %ax;					\
	movw		$0x8f00, %dx;					\
	movl		%eax, (%edi);					\
	movl		%edx, 4(%edi);					\
	addl		$8, %edi

#define LOAD_TRAP_DESC(INTR)						\
	lea		INTR, %edx;					\
	movl		$(KCSEL << 16), %eax;				\
	movw		%dx, %ax;					\
	movw		$0xef00, %dx;					\
	movl		%eax, (%edi);					\
	movl		%edx, 4(%edi);					\
	addl		$8, %edi

	LOAD_INTR_DESC(intr0)
	LOAD_INTR_DESC(intr1)
	LOAD_INTR_DESC(intr2)
	LOAD_INTR_DESC(intr3)
	LOAD_INTR_DESC(intr4)
	LOAD_INTR_DESC(intr5)
	LOAD_INTR_DESC(intr6)
	LOAD_INTR_DESC(intr7)
	LOAD_INTR_DESC(intr8)
	LOAD_INTR_DESC(intr9)

	LOAD_INTR_DESC(intr10)
	LOAD_INTR_DESC(intr11)
	LOAD_INTR_DESC(intr12)
	LOAD_INTR_DESC(intr13)
	LOAD_INTR_DESC(intr14)
	LOAD_INTR_DESC(intr15)
	LOAD_INTR_DESC(intr16)
	LOAD_INTR_DESC(intr17)
	LOAD_INTR_DESC(intr18)
	LOAD_INTR_DESC(intr19)

	LOAD_INTR_DESC(intr20)
	LOAD_INTR_DESC(intr21)
	LOAD_INTR_DESC(intr22)
	LOAD_INTR_DESC(intr23)
	LOAD_INTR_DESC(intr24)
	LOAD_INTR_DESC(intr25)
	LOAD_INTR_DESC(intr26)
	LOAD_INTR_DESC(intr27)
	LOAD_INTR_DESC(intr28)
	LOAD_INTR_DESC(intr29)

	LOAD_INTR_DESC(intr30)
	LOAD_INTR_DESC(intr31)
	LOAD_INTR_DESC(intr32)
	LOAD_INTR_DESC(intr33)
	LOAD_INTR_DESC(intr34)
	LOAD_INTR_DESC(intr35)
	LOAD_INTR_DESC(intr36)
	LOAD_INTR_DESC(intr37)
	LOAD_INTR_DESC(intr38)
	LOAD_INTR_DESC(intr39)

	LOAD_INTR_DESC(intr40)
	LOAD_INTR_DESC(intr41)
	LOAD_INTR_DESC(intr42)
	LOAD_INTR_DESC(intr43)
	LOAD_INTR_DESC(intr44)
	LOAD_INTR_DESC(intr45)
	LOAD_INTR_DESC(intr46)
	LOAD_INTR_DESC(intr47)
	LOAD_TRAP_DESC(intr48)
	LOAD_INTR_DESC(intr49)

	LOAD_INTR_DESC(intr50)
	LOAD_INTR_DESC(intr51)
	LOAD_INTR_DESC(intr52)
	LOAD_INTR_DESC(intr53)
	LOAD_INTR_DESC(intr54)
	LOAD_INTR_DESC(intr55)
	LOAD_INTR_DESC(intr56)
	LOAD_INTR_DESC(intr57)
	LOAD_INTR_DESC(intr58)
	LOAD_INTR_DESC(intr59)

	LOAD_INTR_DESC(intr60)
	LOAD_INTR_DESC(intr61)
	LOAD_INTR_DESC(intr62)
	LOAD_INTR_DESC(intr63)

	/* Start operating system */
        call		os

	/* Not reached */
	hlt

	.align		4, 0x90
__isr:
	pushl		%ebp
	movl		%esp, %ebp

	pushl		%edi
	pushl		%esi
	pushl		%edx
	pushl		%ecx
	pushl		%ebx
	pushl		%eax

	movl		8(%ebp), %eax
	pushl		%eax
	sti
	call		handl
	cli
	addl		$4, %esp

	popl		%eax
	popl		%ebx
	popl		%ecx
	popl		%edx
	popl		%esi
	popl		%edi

	leave
	ret

#define ISR(INTR)							\
	cli;								\
	pushl		%ebp;						\
	movl		%esp, %ebp;					\
	pushl		$INTR;						\
	call		__isr;						\
	addl		$4, %esp;					\
	sti;								\
	leave;								\
	iret;								\
	.align		4, 0x90

	/* Interrupt service routines */
intr0:	ISR(0)
intr1:	ISR(1)
intr2:	ISR(2)
intr3:	ISR(3)
intr4:	ISR(4)
intr5:	ISR(5)
intr6:	ISR(6)
intr7:	ISR(7)
intr8:	ISR(8)
intr9:	ISR(9)

intr10:	ISR(10)
intr11:	ISR(11)
intr12:	ISR(12)
intr13:	ISR(13)
intr14:	ISR(14)
intr15:	ISR(15)
intr16:	ISR(16)
intr17:	ISR(17)
intr18:	ISR(18)
intr19:	ISR(19)

intr20:	ISR(20)
intr21:	ISR(21)
intr22:	ISR(22)
intr23:	ISR(23)
intr24:	ISR(24)
intr25:	ISR(25)
intr26:	ISR(26)
intr27:	ISR(27)
intr28:	ISR(28)
intr29:	ISR(29)

intr30:	ISR(30)
intr31:	ISR(31)
intr32:	ISR(32)
intr33:	ISR(33)
intr34:	ISR(34)
intr35:	ISR(35)
intr36:	ISR(36)
intr37:	ISR(37)
intr38:	ISR(38)
intr39:	ISR(39)

intr40:	ISR(40)
intr41:	ISR(41)
intr42:	ISR(42)
intr43:	ISR(43)
intr44:	ISR(44)
intr45:	ISR(45)
intr46:	ISR(46)
intr47:	ISR(47)

/* System call */
intr48:	
	pushl		%ebp
	movl		%esp, %ebp

	call		_syscall

	leave
	iret
	.align		4, 0x90

intr49:	ISR(49)

intr50:	ISR(50)
intr51:	ISR(51)
intr52:	ISR(52)
intr53:	ISR(53)
intr54:	ISR(54)
intr55:	ISR(55)
intr56:	ISR(56)
intr57:	ISR(57)
intr58:	ISR(58)
intr59:	ISR(59)

intr60:	ISR(60)
intr61:	ISR(61)
intr62:	ISR(62)
intr63:	ISR(63)

	/* Interrupt descriptor table */
idt:
	.space		INTRS * 8

	/* Interrupt descriptor table ptr */
idtptr:
	.short          (INTRS * 8) - 1
        .long           idt
        .align		4, 0

	/* Initial kernel stack space */
        .space		STACK_SIZE
	.globl		initkstk
initkstk:
