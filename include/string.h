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

#ifndef __STRING_H
#define __STRING_H

#include <sys/types.h>

void uint2str(u_int v, char *s, int base);

int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t n);
char *strcat(char *s, const char *append);
char *strncat(char *s, const char *append, size_t n);
size_t strlen(const char *s);
char *strerror(int errno);
size_t strspn(const char *s, const char *accept);
size_t strcspn(const char *s, const char *reject);
const char *strstr(const char *haystack, const char *needle);
const char *strchr(const char *s, int c);
const char *strrchr(const char *s, int c);
int memcmp(const void *s1, const void *s2, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
void *memset(void *b, int c, size_t n);
void *memmove(void *dst, const void *src, size_t n);
void bcopy(const void *src, void *dst, size_t n);
void bzero(void *b, size_t len);

#endif
