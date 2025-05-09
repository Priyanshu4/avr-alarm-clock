/*
 * datetime.c
 *
 * Created: 5/3/2025 2:12:16 PM
 *  Author: agpri
 */ 

#include "datetime.h"
#include <stdio.h>
#include <string.h>

// Static mapping for full day-of-week names (index 0 is invalid)
static const char * const DOW_NAMES[] = {
	"", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

// Static mapping for short day-of-week names
static const char * const DOW_SHORT_NAMES[] = {
	"", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

// Map AM/PM to string
static const char * const AMPM_STRINGS[] = { "AM", "PM" };

void DateTime_FromDS3231Array(const uint8_t data[DATETIME_DS3231_DATA_LENGTH],
DateTime *dt) {
	// DS3231 BCD decode is handled elsewhere; copy raw values
	dt->second    = data[DATETIME_DS3231_REG_SECOND];
	dt->minute    = data[DATETIME_DS3231_REG_MINUTE];
	dt->hour      = data[DATETIME_DS3231_REG_HOUR];
	dt->dayOfWeek = (DateTime_DayOfWeek)data[DATETIME_DS3231_REG_DAY_OF_WEEK];
	dt->day       = data[DATETIME_DS3231_REG_DAY];
	dt->month     = data[DATETIME_DS3231_REG_MONTH];
	dt->year      = data[DATETIME_DS3231_REG_YEAR]; // assume offset

	dt->dateValid = 1;
}

void DateTime_ToDS3231Array(const DateTime *dt,
uint8_t data[DATETIME_DS3231_DATA_LENGTH]) {
	data[DATETIME_DS3231_REG_SECOND]      = dt->second;
	data[DATETIME_DS3231_REG_MINUTE]      = dt->minute;
	data[DATETIME_DS3231_REG_HOUR]        = dt->hour;
	data[DATETIME_DS3231_REG_DAY_OF_WEEK] = (uint8_t)dt->dayOfWeek;
	data[DATETIME_DS3231_REG_DAY]         = dt->day;
	data[DATETIME_DS3231_REG_MONTH]       = dt->month;
	data[DATETIME_DS3231_REG_YEAR]        = dt->year;
}

uint8_t DateTime_Equals(const DateTime *a, const DateTime *b) {
	return (a->second    == b->second  &&
	a->minute    == b->minute  &&
	a->hour      == b->hour    &&
	a->dateValid == b->dateValid &&
	(!a->dateValid ||
	(a->day   == b->day   &&
	a->month == b->month &&
	a->year  == b->year)) &&
	 (a->dayOfWeek == b->dayOfWeek));
}

uint8_t DateTime_TimeEquals(const DateTime *a, const DateTime *b) {
	return (a->hour   == b->hour   &&
	a->minute == b->minute &&
	a->second == b->second);
}

DateTime_Hour12 DateTime_GetHour12(const DateTime *dt) {
	DateTime_Hour12 h12;
	uint8_t h = dt->hour;
	if (h == 0) {
		h12.hour = 12;
		h12.ampm = DateTime_AM;
	} else if (h < 12) {
		h12.hour = h;
		h12.ampm = DateTime_AM;
	} else if (h == 12) {
		h12.hour = 12;
		h12.ampm = DateTime_PM;
	} else {
		h12.hour = h - 12;
		h12.ampm = DateTime_PM;
	}
	return h12;
}

const char *DateTime_AMPMToString(DateTime_AMPM ampm) {
	if (ampm > DateTime_PM) return "";
	return AMPM_STRINGS[ampm];
}

char* DateTime_FormatTime(const DateTime *dt, char *buffer, size_t len, uint8_t twelveHourFmt, uint8_t includeSeconds) {
	size_t required = 0;
	if (twelveHourFmt) {
		// 12-hour format
		if (includeSeconds) {
			// worst-case: "12:MM:SS AM" + '\0' = 12
			required = 12;
		} else {
			// worst-case: "12:MM AM" + '\0' = 9
			required = 9;
		}
	} else {
		// 24-hour format
		if (includeSeconds) {
			// "HH:MM:SS" + '\0' = 9
			required = 9;
		} else {
			// "HH:MM" + '\0' = 6
			required = 6;
		}
	}
	if (len < required) {
		return NULL;
	}

	if (twelveHourFmt) {
		DateTime_Hour12 h12 = DateTime_GetHour12(dt);
		if (includeSeconds) {
			sprintf(buffer, "%u:%02u:%02u %s", h12.hour, dt->minute, dt->second, DateTime_AMPMToString(h12.ampm));
		} else {
			sprintf(buffer, "%u:%02u %s", h12.hour, dt->minute, DateTime_AMPMToString(h12.ampm));
		}
	} 
	else 
	{
		if (includeSeconds) {
			sprintf(buffer, "%02u:%02u:%02u", dt->hour, dt->minute, dt->second);
		} else {
			sprintf(buffer, "%02u:%02u", dt->hour, dt->minute);
		}
	}
	return buffer;
}

char* DateTime_FormatDate(const DateTime *dt, char *buffer, size_t len) {
	// required buffer: "MM/DD/YY" + '\0' = 9
	const size_t required = 9;
	if (len < required) {
		return NULL;
	}
	sprintf(buffer, "%02u/%02u/%02u", dt->month, dt->day, dt->year % 100);
	return buffer;
}

const char* DateTime_DayOfWeekToString(DateTime_DayOfWeek dow) {
	if (dow < DateTime_Sunday || dow > DateTime_Saturday)
	return "";
	return DOW_NAMES[dow];
}

const char* DateTime_DayOfWeekToShortString(DateTime_DayOfWeek dow) {
	if (dow < DateTime_Sunday || dow > DateTime_Saturday)
	return "";
	return DOW_SHORT_NAMES[dow];
}

DateTime DateTime_AddTimeDuration(const DateTime *dt, unsigned int hours, unsigned int minutes, unsigned int seconds) {
	DateTime res = *dt;

	// Local accumulators
	unsigned int hour   = dt->hour;
	unsigned int minute = dt->minute;
	unsigned int second = dt->second;

	// Add seconds, minutes, hours
	second += seconds;
	minute += second / 60;
	second %= 60;

	minute += minutes;
	hour   += minute / 60;
	minute %= 60;

	hour += hours;
	hour %= 24;

	// Update result time fields
	res.hour   = (uint8_t)hour;
	res.minute = (uint8_t)minute;
	res.second = (uint8_t)second;

	// Invalidate date 
	res.dateValid = 0;
	res.dayOfWeek = DateTime_Invalid_Day;
	
	return res;
}

uint8_t DateTime_IsLeapYear(unsigned int year) {
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
        return 1;
    }
    return 0;
}

uint8_t DateTime_DaysInMonth(uint8_t month, uint8_t leap_year) {
    switch (month) {
        case 1:   // January
        case 3:   // March
        case 5:   // May
        case 7:   // July
        case 8:   // August
        case 10:  // October
        case 12:  // December
            return 31;

        case 4:   // April
        case 6:   // June
        case 9:   // September
        case 11:  // November
            return 30;

        case 2:   // February
            return leap_year ? 29 : 28;

        default:
            // Invalid month
            return 0;
    }
}










