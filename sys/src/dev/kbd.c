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

#include <dev/kbd.h>
#include <dev/uart.h>
#include <errno.h>
#include <event.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys.h>
#include <sys/i8255.h>
#include <sys/i8259.h>
#include <sys/intr.h>
#include <sys/ioctl.h>
#include <sys/proc.h>
#include <sys/types.h>

#define SCAN_CODES		0x60

#define SCAN_CODE_LSHFT		0x2a
#define SCAN_CODE_RSHFT		0x36
#define SCAN_CODE_LSREL		0xaa
#define SCAN_CODE_RSREL		0xb6
#define SCAN_CODE_MFII		0xe0
#define SCAN_CODE_ACK		0xfa
#define SCAN_CODE_RESEND	0xfe

#define KBDQ_LENGTH		32
#define KBD_SHIFT		0x01

static int flags = 0;

static int q[KBDQ_LENGTH];
static int h = 0, t = 0;

static int asciishift[SCAN_CODES] = {
	'\0', ESC, '!', '@', '#', '$', '%', '^',
	'&', '*', '(', ')', '_', '+', '\b', '\t',
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
	'O', 'P', '{', '}', '\n', '\0', 'A', 'S',
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
	'"', '~', '\0', '|', 'Z', 'X', 'C', 'V',
	'B', 'N', 'M', '<', '>', '?', '\0', '*',
	'\0', ' ', '\0', '\0', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '7',
	'8', '9', '-', '4', '5', '6', '+', '1',
	'2', '3', '0', '.', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'
};

static int asciinormal[SCAN_CODES] = {
	'\0', ESC, '1', '2', '3', '4', '5', '6',
	'7', '8', '9', '0', '-', '=', '\b', '\t',
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
	'o', 'p', '[', ']', '\n', '\0', 'a', 's',
	'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
	'\'', '`', '\0', '\\', 'z', 'x', 'c', 'v',
	'b', 'n', 'm', ',', '.', '/', '\0', '*',
	'\0', ' ', '\0', '\0', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '7',
	'8', '9', '-', '4', '5', '6', '+', '1',
	'2', '3', '0', '.', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'
};

static int mfii[SCAN_CODES] = {
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',

	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',

	'\0', '\0', '\0', '\0', '\0', NUM_LCK, SCR_LCK, HOME,
	UP, PG_UP, '-', LEFT, '\0', RIGHT, '+', END,
	DOWN, PG_DOWN, INS, DEL, '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'
};

static __inline u_char kbd_read_status()
{
	return inb(I8255_STATUS);
}

static __inline u_char kbd_read_data()
{
	return inb(I8255_PORT_A);
}

static __inline void kbd_wait()
{
	while (kbd_read_status() & I8255_STATUS_INPB) ;
}

static __inline void kbd_write_cmd(u_char val)
{
	outb(I8255_CTRL, val);
}

static void kbd_isr(void *params)
{
	u_char status, code;

	disable;

	status = inb(I8255_STATUS);
	while (status & I8255_STATUS_OUTB) {
		code = inb(I8255_PORT_A);
		if (code == SCAN_CODE_LSHFT || code == SCAN_CODE_RSHFT)
			flags |= KBD_SHIFT;

		else if (code == SCAN_CODE_LSREL || code == SCAN_CODE_RSREL)
			flags &= ~KBD_SHIFT;

		else if (code == SCAN_CODE_MFII) {
			u_char mfii_code;

			/* Wait for next scan code */
			while (!(inb(I8255_STATUS) & I8255_STATUS_OUTB)) ;

			/* Read MF II scan code */
			mfii_code = inb(I8255_PORT_A);
			if (mfii_code > 0 &&
			    mfii_code < SCAN_CODES &&
			    ((t + 1) % KBDQ_LENGTH) != h
			    && mfii[mfii_code] != '\0') {
				q[t] = mfii[mfii_code];
				t = (t + 1) % KBDQ_LENGTH;
			}

		} else if (code > 0 && code < SCAN_CODES) {
			if ((t + 1) % KBDQ_LENGTH != h) {
				if (flags & KBD_SHIFT)
					q[t] = asciishift[code];
				else
					q[t] = asciinormal[code];
				t = (t + 1) % KBDQ_LENGTH;
			}
		}
		status = inb(I8255_STATUS);
	}
	/* Issue keyboard eoi */
	outb(I8259_MSTR_CTRL, I8259_EOI_KBD);

	enable;
}

int kbd_init(void *dev)
{
	isr_inst(INTR_KBD, kbd_isr, NULL);
	intr_unmask(INTR_KBD);

	/* Flush keyboard buffer */
	inb(I8255_PORT_A);
	inb(I8255_PORT_A);

	/* Issue keyboard eoi */
	outb(I8259_MSTR_CTRL, I8259_EOI_KBD);

	return 0;
}

int kbd_shut(void *dev)
{
	return 0;
}

void kbd_flush()
{
	disable;

	/* Flush keyboard buffer */
	h = t = 0;

	/* Flush keyboard buffer */
	inb(I8255_PORT_A);
	inb(I8255_PORT_A);

	/* Issue keyboard eoi */
	outb(I8259_MSTR_CTRL, I8259_EOI_KBD);

	enable;
}

int get()
{
	int c;
#if 0
	disable;

 get_wait:
	while (h == t) {
		enable;
		event_wait(EVENT_KBD);
		disable;
	}
	c = q[h];
	h = (h + 1) % KBDQ_LENGTH;
	if (c == 0)
		goto get_wait;

	enable;
#endif
	c = uart_getchar();

	return c;
}

int kbd_ioctl(void *dev, int cmd, void *args)
{
	if (cmd == GET_BUFFER_SIZE) {
		if (args == NULL)
			return EINVAL;
		*((u_long *) args) = 1;
		return 0;
	}
	return ENOTTY;
}

int kbd_get(void *dev, int *c)
{
	if (c == NULL)
		return EINVAL;

	/* get() handles disabling interrupts */
	*c = get();

	if (*c < 0)
		return *c;
	return 0;
}
