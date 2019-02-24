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

#ifndef __STDIO_H
#define __STDIO_H

#include <stdarg.h>
#include <sys/config.h>
#include <sys/types.h>

#define EOF		(-1)
#define FILENAME_MAX	PATH_LENGTH
#define FOPEN_MAX	FILES
#define STDIN		(-1)
#define STDOUT		(-2)
#define STDERR		(-3)

typedef void FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stdierr;

#if _KERNEL

void print(char *string, int *pos, const char *fmt, va_list args);
void kprintf(const char *fmt, ...);

#endif				/* _KERNEL */

FILE *fopen(const char *path, const char *mode);
FILE *freopen(const char *path, const char *mode, FILE * stream);
int fflush(FILE * stream);
int fclose(FILE * stream);
int remove(const char *path);
int rename(const char *oldname, const char *newname);
FILE *tmpfile();
int fprintf(FILE * stream, const char *fmt, ...);
int printf(const char *fmt, ...);
int sprintf(char *s, const char *fmt, ...);
int vfprintf(FILE * stream, const char *fmt, va_list arg);
int vprintf(const char *fmt, va_list arg);
int vsprintf(char *s, const char *fmt, va_list arg);
int fscanf(FILE * stream, const char *fmt, ...);
int scanf(const char *fmt, ...);
int sscanf(char *s, const char *fmt, ...);
int fgetc(FILE * stream);
char *fgets(char *s, int n, FILE * stream);
int fputc(int c, FILE * stream);
char *fputs(const char *s, FILE * stream);
int getc(FILE * stream);
int getchar();
char *gets(char *s);
int putc(int c, FILE * stream);
int putchar(int c);
int puts(const char *s);
int unget(int c, FILE * stream);
size_t fread(void *ptr, size_t size, size_t nobj, FILE * stream);
size_t fwrite(void *ptr, size_t size, size_t nobj, FILE * stream);
int fseek(FILE * stream, long offset, int origin);
long ftell(FILE * stream);
void rewind(FILE * stream);
int fgetpos(FILE * stream, fpos_t * ptr);
int fsetpos(FILE * stream, const fpos_t * ptr);
void clearerr(FILE * stream);
int feof(FILE * stream);
int ferror(FILE * stream);
void perror(const char *s);

#endif
