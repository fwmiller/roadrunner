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

#ifndef __TIME_H
#define __TIME_H

#include <sys/types.h>

#if _KERNEL

/* 
 * XXX This is really ugly, but it works (tm).  This is a cheesy short-term
 * delay for non-Pentium computers.
 */
#define DELAY(N) { register int n; for (n = 100 * (N); n > 0; n--); }

/* Busy-wait delay with second resolution for non-Pentium computers */
#define delay(N)							\
{									\
    time_t start = time();						\
    while (time() - start < (N));					\
}

#endif				/* _KERNEL */

/* Convenience macros for operations on timevals */
#define timerisset(T) ((T)->tv_sec || (T)->tv_usec)
#define timerclear(T) ((T)->tv_sec = (T)->tv_usec = 0)

#define timernormalize(T)						\
    while ((T)->tv_usec >= 1000000) {					\
	(T)->tv_sec++;							\
	(T)->tv_usec -= 1000000;					\
    }

/* timercmp() works only with the <, >, and == relations */
#define timercmp(T1, T2, CMP)						\
    (((T1)->tv_sec == (T2)->tv_sec) ?					\
     ((T1)->tv_usec CMP (T2)->tv_usec) :				\
     ((T1)->tv_sec CMP (T2)->tv_sec))

/* T1 + T2 = T3 */
#define timeradd(T1, T2, T3)						\
{									\
    (T3)->tv_sec = (T1)->tv_sec + (T2)->tv_sec;				\
    (T3)->tv_usec = (T1)->tv_usec + (T2)->tv_usec;			\
    if ((T3)->tv_usec >= 1000000) {					\
	++(T3)->tv_sec;							\
	(T3)->tv_usec -= 1000000;					\
    }									\
}

/* T1 - T2 = T3 */
#define timersub(T1, T2, T3)						\
{									\
    (T3)->tv_sec = (T1)->tv_sec - (T2)->tv_sec;				\
    (T3)->tv_usec = (T1)->tv_usec - (T2)->tv_usec;			\
    if ((T3)->tv_usec < 0) {						\
	--(T3)->tv_sec;							\
	(T3)->tv_usec += 1000000;					\
    }									\
}

typedef struct timeval {
    long tv_sec;		       /* Seconds */
    long tv_usec;		       /* Microseconds */
} *timeval_t;

struct timezone {
    int tz_minuteswest;		       /* Minutes west of Greenwich */
    int tz_dsttime;		       /* Type of dst correction */
};

struct tm {
    int tm_sec;			       /* Seconds after the minute [0-60] */
    int tm_min;			       /* Minutes after the hour [0-59] */
    int tm_hour;		       /* Hours since midnight [0-23] */
    int tm_mday;		       /* Day of the month [1-31] */
    int tm_mon;			       /* Months since January [0-11] */
    int tm_year;		       /* Years since 1900 */
    int tm_wday;		       /* Days since Sunday [0-6] */
    int tm_yday;		       /* Days since January 1 [0-365] */
    int tm_isdst;		       /* Daylight Savings Time flag */
};

typedef u_long time_t;

#if _KERNEL

extern u_long tick;

void time_init();
time_t time();
void utime(long *tv_sec, long *tv_usec);
void time2str(const time_t clock, char *s);
time_t tm2time(struct tm *tm);

#endif				/* _KERNEL */

void tm2str(const struct tm *tm, char *s);
void time2tm(const time_t clock, struct tm *tm);

#endif
