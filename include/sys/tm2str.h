/* 
 * Based on asctime.c Original Author: G. Haley
 *
 * Converts the broken down time in the structure pointed to by tm into a
 * string of the form:
 *
 * Wed Jun 15 11:38:07 1988\n\0
 *
 */

/*
 * This file provides an generic time representation conversion
 * function that is used both in the kernel and the system library,
 * libsys.a.  The file is included in only two places,
 * roadrunner/libsrc/libsys/time2tm.c and
 * roadrunner/sys/src/time/time2tm.c, which is why the time2tm()
 * function does not use the static definition.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

static const char day_name[7][4] = {
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

static const char mon_name[12][4] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

void
tm2str (const struct tm *tm, char *s)
{
  if (tm == NULL || s == NULL)
    return;

  sprintf (s,
	   "%s %s %2d %2d:%02d:%02d %d\n",
	   day_name[tm->tm_wday],
	   mon_name[tm->tm_mon],
	   tm->tm_mday,
	   tm->tm_hour, tm->tm_min, tm->tm_sec, tm->tm_year + 1900);
}
