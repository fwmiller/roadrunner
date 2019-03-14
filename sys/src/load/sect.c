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
#include <string.h>
#include <sys.h>
#include <sys/load.h>

static int find_section(sections_t s, char *section)
{
	Elf32_Ehdr *ehdr = s->ehdr;
	Elf32_Shdr *shdrtab = s->shdrtab;
	char *shstrtab = s->shstrtab;
	int i;

	for (i = 0; i < ehdr->e_shnum; i++)
		if (strcmp(shstrtab + shdrtab[i].sh_name, section) == 0)
			break;
	if (i == ehdr->e_shnum)
		return (-1);
	return i;
}

int init_sections(char *filebuf, sections_t s)
{
	int i, ndx;

	bzero(s, sizeof(struct sections));

	s->ehdr = (Elf32_Ehdr *) filebuf;
	s->shdrtab = (Elf32_Shdr *) (filebuf + s->ehdr->e_shoff);
	s->shstrtab = filebuf + s->shdrtab[s->ehdr->e_shstrndx].sh_offset;
#if _DEBUG
	dump_ehdr(s);
#endif

	/* Symbol table */
	ndx = find_section(s, SYMTAB);
	s->symtab = (Elf32_Sym *) (filebuf + s->shdrtab[ndx].sh_offset);
	s->symtabentries = s->shdrtab[ndx].sh_size / s->shdrtab[ndx].sh_entsize;

	/* String table */
	ndx = find_section(s, STRTAB);
	s->strtab = filebuf + s->shdrtab[ndx].sh_offset;

	/*
	 * XXX Assume there is at most one occurence of each of the following
	 * sections
	 */
	s->textndx = find_section(s, TEXT);
	s->rodatandx = find_section(s, RODATA);
	s->datandx = find_section(s, DATA);
	s->bssndx = find_section(s, BSS);

	/* TEXT relocations */
	ndx = find_section(s, RELTEXT);
	if (ndx >= 0) {
		s->reltext =
		    (Elf32_Rel *) (filebuf + s->shdrtab[ndx].sh_offset);
		s->reltextentries =
		    s->shdrtab[ndx].sh_size / s->shdrtab[ndx].sh_entsize;
	}

	/* RODATA relocations */
	ndx = find_section(s, RELRODATA);
	if (ndx >= 0) {
		s->relrodata =
		    (Elf32_Rel *) (filebuf + s->shdrtab[ndx].sh_offset);
		s->relrodataentries =
		    s->shdrtab[ndx].sh_size / s->shdrtab[ndx].sh_entsize;
	}

	/* DATA relocations */
	ndx = find_section(s, RELDATA);
	if (ndx >= 0) {
		s->reldata =
		    (Elf32_Rel *) (filebuf + s->shdrtab[ndx].sh_offset);
		s->reldataentries =
		    s->shdrtab[ndx].sh_size / s->shdrtab[ndx].sh_entsize;
	}

	/* Count the common symbols */
	for (i = 0; i < s->symtabentries; i++)
		if (s->symtab[i].st_shndx == SHN_COMMON) {
			s->comcnt++;
			s->comsize += s->symtab[i].st_size;
		}
#if _DEBUG
	if (s->comcnt > 0)
		kprintf("init_sections: common symbols %d size %d\n",
			s->comcnt, s->comsize);
#endif

	return 0;
}

int load_sections(sections_t s, char *filebuf, char *prog)
{
	Elf32_Shdr *shdrtab = s->shdrtab;
	u_long offset;

	s->textoff = (u_long) prog;
#if _DEBUG
	kprintf("load_sections: .text %08x", s->textoff);
#endif
	bcopy(filebuf + shdrtab[s->textndx].sh_offset,
	      (char *)s->textoff, shdrtab[s->textndx].sh_size);
	offset = s->textoff + ALIGN(shdrtab[s->textndx].sh_size, PAGE_SIZE);

	if (s->rodatandx >= 0) {
		s->rodataoff = offset;
#if _DEBUG
		kprintf(" .rodata %08x", s->rodataoff);
#endif
		bcopy(filebuf + shdrtab[s->rodatandx].sh_offset,
		      (char *)s->rodataoff, shdrtab[s->rodatandx].sh_size);
		offset += ALIGN(shdrtab[s->rodatandx].sh_size, PAGE_SIZE);
	}

	if (s->datandx >= 0) {
		s->dataoff = offset;
#if _DEBUG
		kprintf(" .data %08x", s->dataoff);
#endif
		bcopy(filebuf + shdrtab[s->datandx].sh_offset,
		      (char *)s->dataoff, shdrtab[s->datandx].sh_size);
		offset += ALIGN(shdrtab[s->datandx].sh_size, PAGE_SIZE);
	}

	if (s->bssndx >= 0 || s->comsize > 0) {
		s->bssoff = offset;
#if _DEBUG
		kprintf(" .bss %08x", s->bssoff);
#endif
		if (s->comcnt > 0) {
			s->comoff = offset;
			if (s->bssndx >= 0)
				s->comoff += shdrtab[s->bssndx].sh_size;
		}
	}
#if _DEBUG
	kprintf("\n");
#endif

	return 0;
}
