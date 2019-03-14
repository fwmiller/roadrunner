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

#include <stdio.h>
#include <sys.h>
#include <sys/elf.h>
#include <sys/load.h>

static u_long relocate_com(sections_t s, Elf32_Sym * sym)
{
	int i;

	/* Search common table for an existing entry */
	for (i = 0; i < COMMONS; i++)
		if (sym == s->comtab[i].sym)
			break;
	if (i < COMMONS)
		return s->comtab[i].offset;

	/* Allocate an new entry for a common symbol */
	s->comtab[s->comtabndx].sym = sym;
	s->comtab[s->comtabndx].offset = s->comoff;
	s->comoff += ALIGN(sym->st_size, sizeof(u_long));

	return s->comtab[s->comtabndx++].offset;
}

void relocate(sections_t s, u_long offset, Elf32_Rel * rel)
{
	Elf32_Sym *sym;
	u_long *ptr;

	ptr = (u_long *) (offset + rel->r_offset);
	sym = &(s->symtab[ELF32_R_SYM(rel->r_info)]);

	if (ELF32_R_TYPE(rel->r_info) == R_386_32) {
		if (sym->st_shndx == SHN_COMMON)
			*ptr = relocate_com(s, sym);

		else {
			u_long section;

			if (sym->st_shndx == s->textndx)
				section = s->textoff;
			else if (sym->st_shndx == s->rodatandx)
				section = s->rodataoff;
			else if (sym->st_shndx == s->datandx)
				section = s->dataoff;
			else if (sym->st_shndx == s->bssndx)
				section = s->bssoff;
			else {
#if _DEBUG
				kprintf
				    ("relocate: illegal symbol shndx %04x\n",
				     sym->st_shndx);
#endif
			}
			*ptr = section + sym->st_value + *ptr;
		}

	} else if (ELF32_R_TYPE(rel->r_info) == R_386_PC32)
		*ptr = sym->st_value + *ptr - rel->r_offset;

	else {
#if _DEBUG
		kprintf("relocate: unrecognized relocation type (%02x)\n",
			ELF32_R_TYPE(rel->r_info));
#endif
	}
}
