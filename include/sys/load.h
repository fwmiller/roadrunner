/*
 *  Roadrunner/pk
 *    Copyright (C) 1989-2002  Cornfed Systems, Inc.
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

#ifndef __LOAD_H
#define __LOAD_H

#include <sys/elf.h>

#define TEXT		".text"
#define RODATA		".rodata"
#define DATA		".data"
#define BSS		".bss"
#define RELTEXT		".rel.text"
#define RELRODATA	".rel.rodata"
#define RELDATA		".rel.data"
#define SYMTAB		".symtab"
#define STRTAB		".strtab"
#define STARTUP		"__startup"
#define MAIN		"main"

#define COMMONS		256

struct com
{
  Elf32_Sym *sym;
  u_long offset;
};

struct sections
{
  Elf32_Ehdr *ehdr;		/* ELF file header */
  Elf32_Shdr *shdrtab;		/* Section header table */
  char *shstrtab;		/* Section header string table */
  Elf32_Sym *symtab;		/* Symbol table */
  int symtabentries;		/* Number of symbol table entries */
  char *strtab;			/* String table */

  int textndx;			/* TEXT section hdr table index */
  u_long textoff;		/* Address of loaded TEXT section */
  int rodatandx;		/* RODATA section hdr table index */
  u_long rodataoff;		/* Address of loaded RODATA section */
  int datandx;			/* DATA section hdr table index */
  u_long dataoff;		/* Address of loaded DATA section */
  int bssndx;			/* BSS section hdr table index */
  u_long bssoff;		/* Address of BSS section */

  Elf32_Rel *reltext;		/* RELTEXT section */
  int reltextentries;		/* Number of TEXT reloc entries */
  Elf32_Rel *relrodata;		/* RELRODATA section */
  int relrodataentries;		/* Number of RODATA reloc entries */
  Elf32_Rel *reldata;		/* RELDATA section */
  int reldataentries;		/* Number of DATA reloc entries */

  /* Number of common symbols */
  int comcnt;

  /* Size of area needed to hold common symbols */
  int comsize;

  /* Address in BSS section for common symbols */
  u_long comoff;

  /* Index of next free entry in common symbols table */
  int comtabndx;

  /* Table of common symbols that are relocated to the BSS section */
  struct com comtab[COMMONS];
};

typedef struct sections *sections_t;

void dump_ehdr (sections_t s);
void dump_shdrtab (sections_t s);
void dump_sym (sections_t s, Elf32_Sym * sym);
void dump_symtab (sections_t s);
void dump_reltab (sections_t s, Elf32_Rel * reltab, int entries);
int init_sections (char *filebuf, sections_t s);
int load_sections (sections_t s, char *filebuf, char *prog);
void relocate (sections_t s, u_long offset, Elf32_Rel * rel);

#endif
