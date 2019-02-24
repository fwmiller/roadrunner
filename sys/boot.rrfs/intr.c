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

#include <sys.h>
#include <sys/i8259.h>

void
intrinit()
{
    outb(I8259_MSTR_CTRL, I8259_MSTR_ICW1);
    outb(I8259_SLV_CTRL, I8259_SLV_ICW1);
    outb(I8259_MSTR_MASK, I8259_MSTR_ICW2);
    outb(I8259_SLV_MASK, I8259_SLV_ICW2);
    outb(I8259_MSTR_MASK, I8259_MSTR_ICW3);
    outb(I8259_SLV_MASK, I8259_SLV_ICW3);
    outb(I8259_MSTR_MASK, I8259_MSTR_ICW4);
    outb(I8259_SLV_MASK, I8259_SLV_ICW4);
    outb(I8259_MSTR_MASK, I8259_MSTR_DISABLE);
    outb(I8259_SLV_MASK, I8259_SLV_DISABLE);
}

void
intrunmask(int intno)
{
    u_char mask;

    if (intno >= 32 && intno < 40) {
	mask = inb(I8259_MSTR_MASK);
	mask &= ~(0x01 << (intno - 32));
	outb(I8259_MSTR_MASK, mask);

    } else if (intno >= 40 && intno < 48) {
	mask = inb(I8259_SLV_MASK);
	mask &= ~(0x01 << (intno - 40));
	outb(I8259_SLV_MASK, mask);
    }
}
