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
#include <stdlib.h>
#include <string.h>
#include <sys/intr.h>
#include <sys/time.h>
#include <sys/timer.h>

#define TIMER_NAME_LEN	32
#define TIMERS		16

typedef struct timer {
    char name[TIMER_NAME_LEN];
    struct timeval tv;
    struct timeval period;
    timer_func_t f;
    void *arg;
    struct timer *next;
} *timer_t;

static struct timer timertab[TIMERS];
static timer_t timers = NULL;

static void
timer_clear(timer_t timer)
{
    bzero(timer->name, TIMER_NAME_LEN);
    timerclear(&(timer->tv));
    timerclear(&(timer->period));
    timer->f = NULL;
    timer->arg = NULL;
    timer->next = NULL;
}

static void
timer_insert(timer_t timer)
{
    timer_t t;

    if (timers == NULL) {
	timers = timer;
	return;
    }
    /* Insert new timer at head of list */
    if (timercmp(&(timer->tv), &(timers->tv), <)) {
	timer->next = timers;
	timers = timer;
	return;
    }

    /* Insert new timer somewhere after the head of the list */
    for (t = timers; t->next != NULL; t = t->next)
	if (timercmp(&(timer->tv), &(t->next->tv), <))
	    break;
    timer->next = t->next;
    t->next = timer;
}

/*
 * This interrupt service routine is called by the clock isr.  Assume
 * interrupts are disabled and cannot be reenabled.
 */
void
timer_isr()
{
    struct timeval now;
    timer_t t;

    utime(&(now.tv_sec), &(now.tv_usec));

    for (; timers != NULL;) {
	if (timercmp(&now, &(timers->tv), <))
	    break;
	(*(timers->f)) (timers->arg);
	t = timers;
	timers = timers->next;

	if (timerisset(&(t->period))) {
	    timeradd(&(t->tv), &(t->period), &(t->tv));
	    timer_insert(t);
	} else {
	    timer_clear(t);
	}
    }
}

void
timer_init()
{
    int i;

    for (i = 0; i < TIMERS; i++)
	timer_clear(&(timertab[i]));
}

static timer_t
timer_alloc()
{
    int i;

    for (i = 0; i < TIMERS; i++)
	if (timertab[i].f == NULL)
	    break;
    if (i == TIMERS)
	return NULL;
    return &(timertab[i]);
}

int
timer_start(char *name, timeval_t dur, timer_func_t f, void *arg)
{
    struct timeval now;
    timer_t t;

    if (dur->tv_sec == 0 && dur->tv_usec == 0)
	return EINVAL;

    disable;

    t = timer_alloc();
    if (t == NULL) {
	enable;
	return EAGAIN;
    }
    t->f = f;
    t->arg = arg;

    /* Compute expiration time of new timer */
    utime(&(now.tv_sec), &(now.tv_usec));
    timeradd(&now, dur, &(t->tv));

    timer_insert(t);

    enable;
    return 0;
}

int
timer_loop(char *name, timeval_t period, timer_func_t f, void *arg)
{
    struct timeval now;
    timer_t t;

    if (period->tv_sec == 0 && period->tv_usec == 0)
	return EINVAL;

    disable;

    t = timer_alloc();
    if (t == NULL) {
	enable;
	return EAGAIN;
    }
    t->f = f;
    t->arg = arg;
    t->period.tv_sec = period->tv_sec;
    t->period.tv_usec = period->tv_usec;

    /* Compute expiration time of new timer */
    utime(&(now.tv_sec), &(now.tv_usec));
    timeradd(&now, period, &(t->tv));

    timer_insert(t);

    enable;
    return 0;
}

int
timer_cancel(char *name)
{
    return ENOSYS;
}
