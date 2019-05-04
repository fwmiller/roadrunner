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

#include <errno.h>
#include <fcntl.h>
#include <fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys.h>
#include <sys/ioctl.h>
#include <sys/load.h>

static int start_sym_search(sections_t s, char *start_sym, char **start)
{
	int i;

	for (i = 1; i < s->symtabentries; i++)
		if (strcmp(s->strtab + s->symtab[i].st_name, start_sym) == 0) {
			*start = (char *)(s->textoff + s->symtab[i].st_value);
			return 1;
		}
#if _DEBUG
	kprintf("start_sym_search: %s not found\n", start_sym);
#endif
	return 0;
}

int load(char *path, char **prog, u_long * size, char **start)
{
	file_t file;
	u_long filesize;
	char *filebuf;
	struct sections s;
	int i, result, undefcnt;

	if (path == NULL || prog == NULL || size == NULL || start == NULL)
		return EINVAL;

	result = file_open(path, O_RDONLY, &file);
	if (result < 0) {
#if _DEBUG
		kprintf("load: could not open %s (%s)\n", path,
			strerror(result));
#endif
		return result;
	}

	result = file_ioctl(file, GET_FILE_SIZE, &filesize);
	if (result < 0) {
#if _DEBUG
		kprintf("load: could not get file size (%s)\n",
			strerror(result));
#endif
		file_close(file);
		return result;
	}
#if _DEBUG
	kprintf("load: file size %d bytes\n", filesize);
#endif

	filebuf = (char *)malloc(filesize);
	if (filebuf == NULL) {
#if _DEBUG
		kprintf("load: could not allocate file buffer\n");
#endif
		file_close(file);
		return ENOMEM;
	}
	/* XXX Read program file into memory in its entirety */
	result = file_read(file, filebuf, (int *)&filesize);
	if (result < 0) {
#if _DEBUG
		kprintf("load: could not read program file (%s)\n",
			strerror(result));
#endif
		free(filebuf);
		file_close(file);
		return result;
	}
	file_close(file);

	/* Determine program section information */
	result = init_sections(filebuf, &s);

	/* Search for undefined symbols */
	for (undefcnt = 0, i = 1; i < s.symtabentries; i++)
		if (s.symtab[i].st_shndx == SHN_UNDEF) {
			if (undefcnt++ == 0)
				kprintf("load: undefined symbols:");
			kprintf(" %s", s.strtab + s.symtab[i].st_name);
		}
	if (undefcnt > 0) {
		kprintf("\n");
		return ENOEXEC;
	}

	/* Determine amount of memory needed to load the program */
	*size = ALIGN(s.shdrtab[s.textndx].sh_size, PAGE_SIZE);
	if (s.rodatandx >= 0)
		*size += ALIGN(s.shdrtab[s.rodatandx].sh_size, PAGE_SIZE);
	if (s.datandx >= 0)
		*size += ALIGN(s.shdrtab[s.datandx].sh_size, PAGE_SIZE);

	if (s.bssndx >= 0)
		*size +=
		    ALIGN(s.shdrtab[s.bssndx].sh_size + s.comsize, PAGE_SIZE);
	else if (s.comcnt > 0)
		*size += ALIGN(s.comsize, PAGE_SIZE);

	/* Allocate and zero program memory */
	*prog = malloc(*size);
	if (*prog == NULL) {
#if _DEBUG
		kprintf("load: could not allocate program memory\n");
#endif
		free(filebuf);
		return ENOMEM;
	}
	bzero(*prog, *size);

	/* Load program sections into their executable locations */
	load_sections(&s, filebuf, *prog);

	/* Find start symbol */
	if (!start_sym_search(&s, STARTUP, start)) {
#if _DEBUG
		kprintf("load: no start symbol\n");
#endif
		free(filebuf);
		return ENOEXEC;
	}

	/* Perform .text relocations */
	if (s.reltextentries > 0) {
		for (i = 0; i < s.reltextentries; i++)
			relocate(&s, s.textoff, &(s.reltext[i]));
	}

	/* Perform .rodata relocations */
	if (s.relrodataentries > 0) {
		for (i = 0; i < s.relrodataentries; i++)
			relocate(&s, s.rodataoff, &(s.relrodata[i]));
	}

	/* Perform .data relocations */
	if (s.reldataentries > 0) {
		for (i = 0; i < s.reldataentries; i++)
			relocate(&s, s.dataoff, &(s.reldata[i]));
	}
	free(filebuf);
	return 0;
}
