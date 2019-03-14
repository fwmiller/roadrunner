/* 
 * Based on mktime.c Original Author: G. Haley
 *
 * Converts the broken-down time, expressed as local time, in the structure
 * pointed to by tm into a calendar time value. The original values of the
 * tm_wday and tm_yday fields of the structure are ignored, and the original
 * values of the other fields have no restrictions. On successful completion
 * the fields of the structure are set to represent the specified calendar
 * time. Returns the specified calendar time. If the calendar time can not be
 * represented, returns the value (time_t) -1.
 */

#include <stdlib.h>
#include <sys/time.h>

#define _SEC_IN_MINUTE 60
#define _SEC_IN_HOUR 3600
#define _SEC_IN_DAY 86400

#define _DAYS_IN_MONTH(x) ((x == 1) ? days_in_feb : DAYS_IN_MONTH[x])
#define _DAYS_IN_YEAR(year) (((year) % 4) ? 365 : 366)

static const int DAYS_IN_MONTH[12] =
    { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static const int _DAYS_BEFORE_MONTH[12] =
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

static void validate_structure(struct tm *tm)
{
	div_t res;
	int days_in_feb = 28;

	/* Calculate time & date to account for out of range values */
	if (tm->tm_sec < 0 || tm->tm_sec > 59) {
		res = div(tm->tm_sec, 60);
		tm->tm_min += res.quot;
		if ((tm->tm_sec = res.rem) < 0)
			tm->tm_sec += 60;
	}
	if (tm->tm_min < 0 || tm->tm_min > 59) {
		res = div(tm->tm_min, 60);
		tm->tm_hour += res.quot;
		if ((tm->tm_min = res.rem) < 0)
			tm->tm_min += 60;
	}
	if (tm->tm_hour < 0 || tm->tm_hour > 23) {
		res = div(tm->tm_hour, 24);
		tm->tm_mday += res.quot;
		if ((tm->tm_hour = res.rem) < 0)
			tm->tm_hour += 24;
	}
	if (tm->tm_mon > 11) {
		res = div(tm->tm_mon, 12);
		tm->tm_year += res.quot;
		if ((tm->tm_mon = res.rem) < 0)
			tm->tm_mon += 12;
	}
	if (_DAYS_IN_YEAR(tm->tm_year) == 366)
		days_in_feb = 29;

	if (tm->tm_mday < 0) {
		while (tm->tm_mday < 0) {
			tm->tm_mday += _DAYS_IN_MONTH(tm->tm_mon);
			if (--tm->tm_mon == -1) {
				tm->tm_year--;
				tm->tm_mon = 12;
				days_in_feb =
				    ((_DAYS_IN_YEAR(tm->tm_year) ==
				      366) ? 29 : 28);
			}
		}
	} else {
		while (tm->tm_mday > _DAYS_IN_MONTH(tm->tm_mon)) {
			tm->tm_mday -= _DAYS_IN_MONTH(tm->tm_mon);
			if (++tm->tm_mon == 12) {
				tm->tm_year++;
				tm->tm_mon = 0;
				days_in_feb =
				    ((_DAYS_IN_YEAR(tm->tm_year) ==
				      366) ? 29 : 28);
			}
		}
	}
}

time_t tm2time(struct tm *tm)
{
	time_t tim = 0;
	long days = 0;
	int year;

	/* Validate structure */
	validate_structure(tm);

	/* Compute hours, minutes, seconds */
	tim += tm->tm_sec + (tm->tm_min * _SEC_IN_MINUTE) +
	    (tm->tm_hour * _SEC_IN_HOUR);

	/* Compute days in year */
	days += tm->tm_mday - 1;
	days += _DAYS_BEFORE_MONTH[tm->tm_mon];
	if (tm->tm_mon > 1 && _DAYS_IN_YEAR(tm->tm_year) == 366)
		days++;

	/* Compute day of the year */
	tm->tm_yday = days;

	if (tm->tm_year > 10000 || tm->tm_year < -10000) {
		return (time_t) - 1;
	}
	/* Compute days in other years */
	if (tm->tm_year > 70) {
		for (year = 70; year < tm->tm_year; year++)
			days += _DAYS_IN_YEAR(year);
	} else if (tm->tm_year < 70) {
		for (year = 69; year > tm->tm_year; year--)
			days -= _DAYS_IN_YEAR(year);
		days -= _DAYS_IN_YEAR(year);
	}
	/* Compute day of the week */
	if ((tm->tm_wday = (days + 4) % 7) < 0)
		tm->tm_wday += 7;

	/* Compute total seconds */
	tim += (days * _SEC_IN_DAY);

	return tim;
}
