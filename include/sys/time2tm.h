/* 
 * localtime_r.c Original Author: Adapted from tzcode maintained by Arthur
 * David Olson.
 *
 * Converts the calendar time pointed to by clock into a broken-down time
 * expressed as local time. Returns a pointer to a structure containing the
 * broken-down time.
 */

/*
 * This file provides an generic time representation conversion
 * function that is used both in the kernel and the system library,
 * libsys.a.  The file is included in only two places,
 * roadrunner/libsrc/libsys/time2tm.c and
 * roadrunner/sys/src/time/time2tm.c, which is why the time2tm()
 * function does not use the static definition.
 */

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define SECSPERMIN	60L
#define MINSPERHOUR	60L
#define HOURSPERDAY	24L
#define SECSPERHOUR	(SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY	(SECSPERHOUR * HOURSPERDAY)
#define DAYSPERWEEK	7
#define MONSPERYEAR	12

#define YEAR_BASE	1900
#define EPOCH_YEAR      1970
#define EPOCH_WDAY      4

#define isleap(y) ((((y) % 4) == 0 && ((y) % 100) != 0) || ((y) % 400) == 0)

static const int mon_lengths[2][MONSPERYEAR] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

static const int year_lengths[2] = { 365, 366 };

void
time2tm(const time_t clock, struct tm *tm)
{
    long days, rem;
    int y;
    int yleap;
    const int *ip;

    if (tm == NULL)
	return;

    bzero(tm, sizeof(struct tm));

    days = ((long) clock) / SECSPERDAY;
    rem = ((long) clock) % SECSPERDAY;
    while (rem < 0) {
	rem += SECSPERDAY;
	--days;
    }
    while (rem >= SECSPERDAY) {
	rem -= SECSPERDAY;
	++days;
    }

    /* Compute hour, min, and sec */
    tm->tm_hour = (int) (rem / SECSPERHOUR);
    rem %= SECSPERHOUR;
    tm->tm_min = (int) (rem / SECSPERMIN);
    tm->tm_sec = (int) (rem % SECSPERMIN);

    /* Compute day of week */
    if ((tm->tm_wday = ((EPOCH_WDAY + days) % DAYSPERWEEK)) < 0)
	tm->tm_wday += DAYSPERWEEK;

    /* Compute year & day of year */
    y = EPOCH_YEAR;
    if (days >= 0) {
	for (;;) {
	    yleap = isleap(y);
	    if (days < year_lengths[yleap])
		break;
	    y++;
	    days -= year_lengths[yleap];
	}
    } else {
	do {
	    --y;
	    yleap = isleap(y);
	    days += year_lengths[yleap];
	}
	while (days < 0);
    }

    tm->tm_year = y - YEAR_BASE;
    tm->tm_yday = days;
    ip = mon_lengths[yleap];
    for (tm->tm_mon = 0; days >= ip[tm->tm_mon]; ++tm->tm_mon)
	days -= ip[tm->tm_mon];
    tm->tm_mday = days + 1;

    /* Set daylight saving time flag */
    tm->tm_isdst = -1;
}
