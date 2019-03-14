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

#include <stdlib.h>
#include <sys/config.h>
#include <sys/mem.h>

region_t valid_region(void *start)
{
	region_t r;

	for (r = alloclist; r != NULL && r->start != (u_long) start;
	     r = r->next) ;
	return r;
}

void region_clear(region_t r)
{
	r->prev = NULL;
	r->next = NULL;
	r->start = 0;
	r->len = 0;
	r->proc = (-1);
}

static inline void region_insert_before(region_t r, region_t s, region_t * l)
{
	r->next = s;
	if (*l == s)
		*l = r;
	else {
		r->prev = s->prev;
		s->prev->next = r;
	}
	s->prev = r;
}

static inline void region_insert_after(region_t r, region_t s)
{
	r->prev = s;
	s->next = r;
}

void region_insert(region_t r, region_t * l)
{
	region_t s;

	if (*l == NULL) {
		*l = r;
		return;
	}
	for (s = *l;; s = s->next) {
		if (s->start > r->start) {
			region_insert_before(r, s, l);
			return;
		}
		if (s->next == NULL) {
			region_insert_after(r, s);
			return;
		}
	}
}

void region_remove(region_t r, region_t * l)
{
	if (r->next != NULL)
		r->next->prev = r->prev;
	if (r->prev == NULL)
		*l = r->next;
	else
		r->prev->next = r->next;
	r->prev = NULL;
	r->next = NULL;
}

void *region_split(region_t r, size_t size)
{
	region_t s;
	int i;

	/* 
	 * Search for a free region table entry to record the split off portion
	 * of the specified region
	 */
	for (i = 0; i < REGIONS && (s = &(regiontab[i]))->start != 0; i++) ;
	if (i == REGIONS)
		return NULL;

	/* Record the split off portion */
	s->start = r->start + size;
	s->len = r->len - size;

	/* Reduce the size of the specified region */
	r->len = size;

	/* 
	 * Enter the split off region the same region list as that containing the
	 * specified region
	 */
	s->prev = r;
	s->next = r->next;
	if (r->next != NULL)
		r->next->prev = s;
	r->next = s;

	/* Return a pointer to start of the split off region */
	return (void *)s->start;
}
