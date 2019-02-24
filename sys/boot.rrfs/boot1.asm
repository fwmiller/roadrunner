;
;  Roadrunner/pk
;    Copyright (C) 1989-2001  Cornfed Systems, Inc.
;
;  The Roadrunner/pk operating system is free software; you can
;  redistribute and/or modify it under the terms of the GNU General
;  Public License, version 2, as published by the Free Software
;  Foundation.
; 
;  This program is distributed in the hope that it will be useful,
;  but WITHOUT WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public
;  License along with this program; if not, write to the Free
;  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
;  MA 02111-1307 USA
;
;  More information about the Roadrunner/pk operating system of
;  which this file is a part is available on the World-Wide Web
;  at: http://www.cornfed.com.
;


	;
	; This boot program is assembled using nasm-0.98
	;
	BITS		16
	SECTION		.text

	; Initial jump to get around the rrfs file system parameters
boot:
	db		0xeb
	db		0x20
	db		0x90
	db		0x90

	; Leave space for rrfs file system parameters
	times 0x22 - $+boot db 0

	;
	; The boot sector is loaded at 0000:7c00 but we want to change the
	; code segment to be 07c0:0000.  This long jump adjusts cs.
	;
	jmp word	0x07c0:start
start:
	; Setup initial environment
	mov		ax, cs
	mov		ds, ax

	; Display boot message
	mov		si, bootmsg
	call		print

	; Copy boot code to working memory
	mov		ax, 0x9000
	mov		es, ax
	mov		si, 0
	mov		di, 0
	mov		cx, 0x100
	rep		movsw

	; Jump to boot code in working memory
	jmp word	0x9000:work
work:
	; Setup working environment
	mov		ax, cs
	mov		ds, ax

	; Setup execution stack
	mov		ax, 0x1000
	mov		ss, ax
	mov		sp, 0xfff8

	; Turn off BIOS cursor...
	mov		ah, 1
	mov		ch, 0x20
	int		0x10

	; ...twice for good measure
	mov		ah, 1
	mov		ch, 0x20
	int		0x10

	; Zero second stage boot program memory
	xor		ax, ax
	mov		es, ax
	mov		di, 0x2000
	; 16 sectors * 512 bytes/sector
	mov		cx, 8192
zeroloop:
	mov byte	[es:di], al
	inc		di
	dec		cx
	cmp		cx, 0
	ja		zeroloop

	; Store boot parameters for kernel
	xor		ax, ax
	mov		es, ax

	; dl contains the drive number left by the bootstrap
	mov		di, 0x1000
	mov byte	[es:di], dl

	; Get BIOS view of the hard disk geometry
	mov		ah, 8
	mov		dl, 0x80
	int		0x13

	; Need to reload es:di after the BIOS operation
	xor		ax, ax
	mov		es, ax
	mov		di, 0x1001

	; Store BIOS view of the hard disk geometry
	mov byte	[es:di], ch	; Low 8 bits of max cyl
	inc		di
	mov byte	[es:di], cl	; Max sec and high 2 bits of max cyl
	inc		di
	mov byte	[es:di], dh	; Max head
	inc		di
	mov byte	[es:di], dl	; Number of drives

	; Reload drive number in dl
	mov		di, 0x1000
	mov		dl, [es:di]

	; Boot forks based on whether drive is a floppy or hard disk
	cmp		dl, 0x80
	jae short	wd

	; Reset floppy disk
	xor		ah, ah
	int		0x13

	; Load second stage boot program
	mov		al, 15		; Number of sectors
	mov		ah, 2
	mov		bx, 0x2000	; Buffer offset
	mov		cl, 2		; Start sec
	xor		ch, ch		; Start cyl
	; dl contains the drive number left by the bootstrap
	xor		dh, dh		; Start head
	int		0x13

	jmp short	getvbeinfo
wd:
	; Load partition table
	mov		ax, 0x1000
	mov		es, ax
	mov		ax, 0x0201
	xor		bx, bx
	mov		cx, 1
	and		dx, 0xff
	int		0x13
	jb short	readerr

	; Search for first rrfs partition
	mov		bx, 0x01be
	mov		di, bx
	add		di, 4
	mov		cx, 4
again:
	mov		al, [es:di]
	cmp		al, 0xcc
	je short	found
	add		bx, 0x10
	add		di, 0x10
	loop		again
	jmp short	parterr
found:
	; Store partition offset with boot parameters for kernel
	add		di, 4
	xor		ax, ax
	mov		fs, ax
	mov		si, 0x1005
	mov		ax, [es:di]
	mov word	[fs:si], ax
	add		si, 2
	add		di, 2
	mov		ax, [es:di]
	mov word	[fs:si], ax

	; Get location of first sector in partition
	mov		di, bx
	inc		di
	mov		dh, [es:di]	; Start head
	inc		di
	mov		cx, [es:di]	; Start cyl and sec

	; Load second stage boot program
	xor		ax, ax
	mov		es, ax
	mov		al, 16		; Number of sectors
	mov		ah, 2
	mov		bx, 0x1e00	; Buffer offset
	int		0x13

	; Obtain VBE controller information
getvbeinfo:
	xor		ax, ax
	mov		es, ax
	mov		di, 0x100a
	mov		ax, 0x4f00
	int		0x10

	; Start second stage boot program
	jmp word	0:0x2000

readerr:
	mov		si, readfailed
	call		print
	jmp short	loopforever

parterr:
	mov		si, nopart
	call		print

loopforever:
	jmp short	loopforever


	; Print string to console
	; si = ptr to first character of a null terminated string
print:
	push		ax
	cld
nextchar:
	mov		al, [si]
	cmp		al, 0
	je short	printdone
	call		printchar
	inc		si
	jmp short	nextchar
printdone:
	pop		ax
	ret


	; Print a single character to the console
	; al = character to be printed
printchar:
	mov		ah, 0x0e
	int		0x10
	ret


	; Message strings
bootmsg:
	db		'Roadrunner boot'
	db		0
readfailed:
	db		'\nread failed'
	db		0
nopart:
	db		'\nno rrfs partition'
	db		0


	; Boot signature
        resb		0x01fe + $$ - $
        db		0x55
        db		0xaa
