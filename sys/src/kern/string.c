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
#include <errno.h>
#include <string.h>
#include <sys/errstr.h>
#include <sys/types.h>

char *
strerror(int errno)
{
    if (errno == 0)
	return errstr[0];

    if (errno > 0 || errno < EMIN)
	return errstr[-(EMIN - 1)];
    return errstr[-errno];
}

void
uint2str(u_int v, char *s, int base)
{
    if (v == 0) {
	s[0] = '0';
	s[1] = '\0';
    } else {
	char s1[80];
	u_int v1;
	int i = 0, j = 0;

	while (v > 0) {
	    v1 = v % base;
	    if (v1 < 10)
		s1[i++] = v1 + '0';
	    else
		s1[i++] = v1 - 10 + 'a';
	    v /= base;
	}
	while (i > 0)
	    s[j++] = s1[--i];
	s[j] = '\0';
    }
}

/* Based on _strtol_r() from newlib */
#define LONG_MIN	(-2147483647 - 1)
#define LONG_MAX	2147483647
static u_int
str2uint(const char *s)
{
    register unsigned long acc;
    register int c;
    register unsigned long cutoff;
    register int neg = 0, any, cutlim;
    register int base = 0;

    /* 
     * Skip white space and pick up leading +/- sign if any. If base is 0,
     * allow 0x for hex and 0 for octal, else assume decimal
     */
    do
	c = *s++;
    while (isspace(c));
    if (c == '-') {
	neg = 1;
	c = *s++;
    } else if (c == '+')
	c = *s++;

    if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X')) {
	c = s[1];
	s += 2;
	base = 16;
    }
    if (base == 0)
	base = c == '0' ? 8 : 10;

    /* 
     * Compute the cutoff value between legal numbers and illegal numbers.
     * That is the largest legal value, divided by the base.  An input number
     * that is greater than this value, if followed by a legal input
     * character, is too big.  One that is equal to this value may be valid
     * or not; the limit between valid and invalid numbers is then based on
     * the last digit.  For instance, if the range for longs is
     * [-2147483648..2147483647] and the input base is 10, cutoff will be set
     * to 214748364 and cutlim to either 7 (neg==0) or 8 (neg==1), meaning
     * that if we have accumulated a value > 214748364, or equal but the next
     * digit is > 7 (or 8), the number is too big, and we will return a range
     * error.
     * 
     * Set any if any `digits' consumed; make it negative to indicate overflow.
     */
    cutoff = neg ? -(unsigned long) LONG_MIN : LONG_MAX;
    cutlim = cutoff % (unsigned long) base;
    cutoff /= (unsigned long) base;
    for (acc = 0, any = 0;; c = *s++) {
	if (isdigit(c))
	    c -= '0';
	else if (isalpha(c))
	    c -= isupper(c) ? 'A' - 10 : 'a' - 10;
	else
	    break;
	if (c >= base)
	    break;
	if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
	    any = (-1);
	else {
	    any = 1;
	    acc *= base;
	    acc += c;
	}
    }
    if (any < 0)
	acc = neg ? LONG_MIN : LONG_MAX;
    else if (neg)
	acc = -acc;

    return (u_int) acc;
}

int
atoi(const char *nptr)
{
    return (int) str2uint(nptr);
}

int
strcmp(const char *s1, const char *s2)
{
    int i = 0;

    for (;;) {
	if (s1[i] == '\0' && s2[i] == '\0')
	    return 0;
	else if (s1[i] == '\0' && s2[i] != '\0')
	    return (-1);
	else if (s1[i] != '\0' && s2[i] == '\0')
	    return 1;

	if (s1[i] == s2[i])
	    i++;
	else if (s1[i] < s2[i])
	    return (-1);
	else
	    return 1;
    }
}

int
strncmp(const char *s1, const char *s2, size_t n)
{
    int i = 0;

    while (i < n) {
	if (s1[i] == '\0' && s2[i] == '\0')
	    return 0;
	else if (s1[i] == '\0' && s2[i] != '\0')
	    return (-1);
	else if (s1[i] != '\0' && s2[i] == '\0')
	    return 1;

	if (s1[i] == s2[i])
	    i++;
	else if (s1[i] < s2[i])
	    return (-1);
	else
	    return 1;
    }
    return 0;
}

char *
strcpy(char *dst, const char *src)
{
    int i;

    for (i = 0; src[i] != '\0'; i++)
	dst[i] = src[i];
    dst[i] = '\0';
    return dst;
}

char *
strncpy(char *dst, const char *src, size_t n)
{
    int i;

    for (i = 0; i < n; i++) {
	dst[i] = src[i];
	if (src[i] == '\0')
	    break;
    }
    return dst;
}

char *
strcat(char *s, const char *append)
{
    strcpy(s + strlen(s), append);
    return s;
}

size_t
strlen(const char *s)
{
    int i = 0;

    while (s[i] != '\0')
	i++;
    return (size_t) i;
}

int
memcmp(const void *ptr1, const void *ptr2, size_t len)
{
    char *s1 = (char *) ptr1;
    char *s2 = (char *) ptr2;
    int i = 0;

    while (i < (int) len)
	if (s1[i] == s2[i])
	    i++;
	else if (s1[i] < s2[i])
	    return -1;
	else
	    return 1;

    return 0;
}

void *
memcpy(void *dst, const void *src, size_t n)
{
    bcopy(src, dst, n);
    return dst;
}

void *
memset(void *b, int c, size_t len)
{
    int i;

    for (i = 0; i < (int) len; i++)
	((char *) b)[i] = c;
    return b;
}

void
bcopy(const void *src, void *dst, size_t len)
{
    int i;

    for (i = 0; i < (int) len; i++)
	((char *) dst)[i] = ((char *) src)[i];
}

void
bzero(void *b, size_t len)
{
    int i;

    for (i = 0; i < (int) len; i++)
	((char *) b)[i] = 0;
}
