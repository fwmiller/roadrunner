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

#include <ctype.h>
#include <dev/cons.h>
#include <dev/uart.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/config.h>
#include <sys/intr.h>
#include <sys/ioctl.h>

#define SCR_BASE	((char *) 0xb8000)
#define SCR_END		((char *) SCR_BASE + ((LINES * COLS) << 1))

/* Cursor flags */
#define CF_CURSOR_OFF	0x01
#define CF_BRIGHT	0x02
#define CF_REVERSE	0x04
#define CF_BLINK	0x08

/* Cursor attributes */
#define CA_NORMAL	0x07
#define CA_BRIGHT	0x0f
#define CA_REVERSE	0x70
#define CA_BLINK	0x87

#define CURSOR		' '

static int scr_flags = 0;
static char *scr_cursor;

static inline void cursor_off()
{
	*(scr_cursor + 1) = CA_NORMAL;
	*scr_cursor = ' ';
}

static inline void cursor_on()
{
	*scr_cursor = CURSOR;
	*(scr_cursor + 1) = CA_REVERSE;
}

void clear_screen()
{
	for (scr_cursor = SCR_BASE; scr_cursor < SCR_END; scr_cursor += 2) {
		*scr_cursor = ' ';
		*(scr_cursor + 1) = CA_NORMAL;
	}
	scr_cursor = SCR_BASE;
	cursor_on();
}

#if 0
static void scroll_screen()
{
	char *i = SCR_BASE;
	char *j = SCR_BASE + 2 * COLS;

	for (; j < SCR_END; i += 2 * COLS, j += 2 * COLS)
		bcopy(j, i, 2 * COLS);
	for (; i < SCR_END; i += 2) {
		*(i + 1) = CA_NORMAL;
		*i = ' ';
	}
	scr_cursor = SCR_END - 2 * COLS;

	if (!(scr_flags & CF_CURSOR_OFF))
		cursor_on();
}
#endif

static inline char get_cursor_attr()
{
	if (!(scr_flags & (CF_BRIGHT | CF_REVERSE | CF_BLINK)))
		return CA_NORMAL;
	if (scr_flags & CF_REVERSE)
		return CA_REVERSE;
	if (scr_flags & CF_BRIGHT)
		return CA_BRIGHT;

	/* Must be CF_BLINK */
	return CA_BLINK;
}

void put(char c)
{
#if 0
	if (isprint(c)) {
		if (!(scr_flags & CF_CURSOR_OFF))
			cursor_off();

		*scr_cursor = c;
		*(scr_cursor + 1) = get_cursor_attr();
		scr_cursor += 2;

		if (scr_cursor >= SCR_END)
			scroll_screen();

		if (!(scr_flags & CF_CURSOR_OFF))
			cursor_on();

	} else if (c == '\n') {
		if (!(scr_flags & CF_CURSOR_OFF))
			cursor_off();

		if (scr_cursor < SCR_END - 2 * COLS) {
			char *i;

			for (i = SCR_BASE; i <= scr_cursor; i += 2 * COLS) ;
			scr_cursor = i;
		} else
			scroll_screen();

		if (!(scr_flags & CF_CURSOR_OFF))
			cursor_on();

	} else if (c == '\b' && scr_cursor > SCR_BASE) {
		if (!(scr_flags & CF_CURSOR_OFF))
			cursor_off();
		scr_cursor -= 2;
		if (!(scr_flags & CF_CURSOR_OFF))
			cursor_on();
	}
#endif
	uart_putchar(c);
}

int cons_init(void *dev)
{
	return 0;
}

int cons_shut(void *dev)
{
	return 0;
}

int cons_ioctl(void *dev, int cmd, void *args)
{
	switch (cmd) {
	case GET_BUFFER_SIZE:
		if (args == NULL)
			return EINVAL;
		*((u_long *) args) = 1;
		break;

	case GET_WINDOW_HEIGHT:
		*((int *)args) = LINES;
		break;

	case GET_WINDOW_WIDTH:
		*((int *)args) = COLS;
		break;

	case GET_CURSOR_POS:
		{
			cursor_t c = (cursor_t) args;

			disable;
			c->x = ((scr_cursor - SCR_BASE) >> 1) / COLS;
			c->y = ((scr_cursor - SCR_BASE) >> 1) % COLS;
			enable;
		}
		break;

	case GET_CURSOR_CHAR:
		{
			char *c = (char *)args;

			disable;
			*c = *scr_cursor;
			enable;
		}
		break;

	case SET_CURSOR_ON:
		disable;
		scr_flags &= ~CF_CURSOR_OFF;
		cursor_on();
		enable;
		break;

	case SET_CURSOR_OFF:
		disable;
		scr_flags |= CF_CURSOR_OFF;
		cursor_off();
		enable;
		break;

	case SET_CURSOR_POS:
		{
			cursor_t c = (cursor_t) args;

			if (c->x < 0 || c->x >= COLS || c->y < 0
			    || c->y >= LINES)
				return EINVAL;

			disable;
			scr_cursor = SCR_BASE + 2 * (c->y * COLS + c->x);
			if (!(scr_flags & CF_CURSOR_OFF))
				cursor_on();
			enable;
		}
		break;

	case SET_NORMAL_VIDEO:
		disable;
		scr_flags &= ~(CF_BRIGHT | CF_REVERSE | CF_BLINK);
		enable;
		break;

	case SET_BRIGHT_VIDEO:
		disable;
		scr_flags &= ~(CF_BRIGHT | CF_REVERSE | CF_BLINK);
		scr_flags |= CF_BRIGHT;
		enable;
		break;

	case SET_REVERSE_VIDEO:
		disable;
		scr_flags &= ~(CF_BRIGHT | CF_REVERSE | CF_BLINK);
		scr_flags |= CF_REVERSE;
		enable;
		break;

	case SET_BLINK_VIDEO:
		disable;
		scr_flags &= ~(CF_BRIGHT | CF_REVERSE | CF_BLINK);
		scr_flags |= CF_BLINK;
		enable;
		break;

	case CLEAR:
		disable;
		clear_screen();
		enable;
		break;

	case CLEAR_TO_EOL:
		{
			int i;

			disable;

			for (i = 0;
			     i <
			     (2 * COLS) -
			     ((scr_cursor - SCR_BASE) % (2 * COLS)); i += 2) {
				*(scr_cursor + i) = ' ';
				*(scr_cursor + i + 1) = CA_NORMAL;
			}
			enable;
		}
		break;

	default:
		return ENOTTY;
	}
	return 0;
}

int cons_put(void *dev, int c)
{
	disable;
	put((char)c);
	enable;
	return 0;
}
