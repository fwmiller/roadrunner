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
#include <sys/elf.h>
#include <sys/load.h>

#define SHDR "sect offset   size     addr     align    entsize  name"

void
dump_ehdr(sections_t s)
{
    Elf32_Ehdr *ehdr = s->ehdr;

    kprintf("dump_ehdr: shoff %08x shentsize %d shnum %d shstrndx %d\n",
	    ehdr->e_shoff, ehdr->e_shentsize,
	    ehdr->e_shnum, ehdr->e_shstrndx);
}

void
dump_shdrtab(sections_t s)
{
    Elf32_Ehdr *ehdr = s->ehdr;
    Elf32_Shdr *shdrtab = s->shdrtab;
    char *shstrtab = s->shstrtab;
    Elf32_Shdr *shdr;
    int i;

    kprintf("%s\n", SHDR);
    for (i = 0; i < ehdr->e_shnum; i++) {
	shdr = &(shdrtab[i]);
	kprintf("%4d %08x %08x %08x %08x %08x %s\n",
		i, (u_int) shdr->sh_offset, (u_int) shdr->sh_size,
		(u_int) shdr->sh_addr, (u_int) shdr->sh_addralign,
		(u_int) shdr->sh_entsize, shstrtab + shdr->sh_name);
    }
}

void
dump_sym(sections_t s, Elf32_Sym * sym)
{
    Elf32_Shdr *shdrtab = s->shdrtab;
    char *shstrtab = s->shstrtab;
    char *strtab = s->strtab;

    if (sym->st_name > 0) {

	kprintf("%08x ", (u_int) sym->st_value);

	switch (ELF32_ST_BIND(sym->st_info)) {
	case STB_LOCAL:
	    kprintf("l");
	    break;
	case STB_GLOBAL:
	    kprintf("g");
	    break;
	case STB_WEAK:
	    kprintf("w");
	    break;
	default:
	    kprintf(" ");
	}

	kprintf("  ");

	switch (ELF32_ST_TYPE(sym->st_info)) {
	case STT_OBJECT:
	    kprintf("d");
	    break;
	case STT_FUNC:
	    kprintf("f");
	    break;
	case STT_SECTION:
	    kprintf("s");
	    break;
	case STT_FILE:
	    kprintf("F");
	    break;
	default:
	    kprintf(" ");
	}

	kprintf(" ");

	switch (sym->st_shndx) {
	case SHN_UNDEF:
	    kprintf("UNDEF");
	    break;
	case SHN_ABS:
	    kprintf("*ABS*");
	    break;
	case SHN_COMMON:
	    kprintf("*COM*");
	    break;
	default:
	    kprintf("%s", shstrtab + shdrtab[sym->st_shndx].sh_name);
	}

	kprintf("  %s\n", strtab + sym->st_name);
    }
}

void
dump_symtab(sections_t s)
{
    Elf32_Sym *symtab = s->symtab;
    int symtabentries = s->symtabentries;
    int i;

    for (i = 0; i < symtabentries; i++)
	dump_sym(s, &(symtab[i]));
}

void
dump_reltab(sections_t s, Elf32_Rel * reltab, int entries)
{
    Elf32_Sym *symtab = s->symtab;
    char *strtab = s->strtab;
    Elf32_Rel *rel;
    int i;

    for (i = 0; i < entries; i++) {
	rel = &(reltab[i]);

	kprintf("%08x  ", rel->r_offset);

	switch (ELF32_R_TYPE(rel->r_info)) {
	case R_386_32:
	    kprintf("R_386_32      ");
	    break;
	case R_386_PC32:
	    kprintf("R_386_PC32    ");
	    break;
	case R_386_GOT32:
	    kprintf("R_386_GOT32   ");
	    break;
	case R_386_PLT32:
	    kprintf("R_386_PLT32   ");
	    break;
	case R_386_COPY:
	    kprintf("R_386_COPY    ");
	    break;
	case R_386_GLOB_DAT:
	    kprintf("R_386_GLOB_DAT");
	    break;
	case R_386_JMP_SLOT:
	    kprintf("R_386_JMP_SLOT");
	    break;
	case R_386_RELATIVE:
	    kprintf("R_386_RELATIVE");
	    break;
	case R_386_GOTOFF:
	    kprintf("R_386_GOTOFF  ");
	    break;
	case R_386_GOTPC:
	    kprintf("R_386_GOTPC   ");
	    break;
	default:
	    kprintf("%02x            ", (u_int) ELF32_R_TYPE(rel->r_info));
	}

	kprintf("  %s\n",
		strtab + symtab[ELF32_R_SYM(rel->r_info)].st_name);
    }
}
