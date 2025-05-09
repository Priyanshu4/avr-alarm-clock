/*
 * datetime.h
 *
 * Created: 5/3/2025 2:06:28 PM
 *  Author: agpri
 */ 
#ifndef DATETIME_H
#define DATETIME_H

#include <stdint.h>
#include <stddef.h>

// Days of week, 0 is invalid
typedef enum {
	DateTime_Invalid_Day = 0,
	DateTime_Sunday,
	DateTime_Monday,
	DateTime_Tuesday,
	DateTime_Wednesday,
	DateTime_Thursday,
	DateTime_Friday,
	DateTime_Saturday
} DateTime_DayOfWeek;

// AM/PM indicator for 12-hour clock
typedef enum {
	DateTime_AM = 0,
	DateTime_PM = 1
} DateTime_AMPM;

// 12-hour clock representation
typedef struct {
	uint8_t       hour; // 1–12
	DateTime_AMPM ampm; // AM or PM
} DateTime_Hour12;

// DateTime holds time; date and dow are optional
typedef struct {
	uint8_t second;    // 0–59
	uint8_t minute;    // 0–59
	uint8_t hour;      // 0–23

	uint8_t dateValid; // 1 if day/month/year valid, 0 if unused
	uint8_t day;       // 1–31 (only if dateValid)
	uint8_t month;     // 1–12 (only if dateValid)
	uint16_t year;     // last two digits of year 00-99

	DateTime_DayOfWeek dayOfWeek; 
} DateTime;

// DS3231 register indices (BCD) for 7-byte array
#define DATETIME_DS3231_REG_SECOND      0
#define DATETIME_DS3231_REG_MINUTE      1
#define DATETIME_DS3231_REG_HOUR        2
#define DATETIME_DS3231_REG_DAY_OF_WEEK 3
#define DATETIME_DS3231_REG_DAY         4
#define DATETIME_DS3231_REG_MONTH       5
#define DATETIME_DS3231_REG_YEAR        6
#define DATETIME_DS3231_DATA_LENGTH     7

// Translate a 7-byte DS3231 BCD array into a DateTime
// Arguments:
// - data: pointer to 7-byte input array (BCD)
// - dt:   pointer to output DateTime (dateValid=1, dowValid=1)
void DateTime_FromDS3231Array(const uint8_t data[DATETIME_DS3231_DATA_LENGTH],
DateTime *dt);

// Encode a DateTime into a 7-byte DS3231 BCD array
// Arguments:
// - dt:   pointer to DateTime to encode
// - data: pointer to 7-byte output array (BCD)
void DateTime_ToDS3231Array(const DateTime *dt,
uint8_t data[DATETIME_DS3231_DATA_LENGTH]);

// Compare two DateTime structs (date + time)
// Returns 1 if exactly equal, 0 otherwise
uint8_t DateTime_Equals(const DateTime *a, const DateTime *b);

// Compare only the time portion of two DateTime structs
// Returns 1 if hour, minute, second match, 0 otherwise
uint8_t DateTime_TimeEquals(const DateTime *a, const DateTime *b);

// Convert 24h hour to 12-hour format
// Arguments:
// - dt: pointer to DateTime with hour in 0–23
// Returns DateTime_Hour12 with hour 1–12 and AM/PM
DateTime_Hour12 DateTime_GetHour12(const DateTime *dt);

// Convert AM/PM enum to "AM" or "PM"
// Arguments:
// - ampm: DateTime_AM or DateTime_PM
// Returns pointer to static string "AM" or "PM"
const char *DateTime_AMPMToString(DateTime_AMPM ampm);

// Format time portion of a DateTime into a string
// Arguments:
// - dt:             pointer to source DateTime
// - buffer:         pointer to output buffer
// - len:            buffer length
// - twelveHourFmt:  1 = 12-hour clock, 0 = 24-hour
// - includeSeconds: 1 = include seconds, 0 = omit seconds
// Returns pointer to buffer
char *DateTime_FormatTime(const DateTime *dt, char *buffer, size_t len, uint8_t twelveHourFmt, uint8_t includeSeconds);

// Format date portion of a DateTime into a string in MM/DD/YY format
// Arguments:
// - dt:             pointer to source DateTime
// - buffer:         pointer to output buffer
// - len:            buffer length
// Returns pointer to buffer
char *DateTime_FormatDate(const DateTime *dt, char *buffer, size_t len);

// Full day-of-week name from enum
// Arguments:
// - dow: one of DateTime_DOW_* values
// Returns pointer to static string or empty if invalid
const char *DateTime_DayOfWeekToString(DateTime_DayOfWeek dow);

// Three-letter day-of-week name from enum
// Arguments:
// - dow: one of DateTime_DOW_* values
// Returns pointer to static 3-letter string or empty if invalid
const char *DateTime_DayOfWeekToShortString(DateTime_DayOfWeek dow);

// Add a duration to the time of a DateTime, normalizing all time fields.
// This function does not update the date. It invalidates the date and day of week.
// Arguments:
// - dt: source DateTime
// - hours, minutes, seconds: duration
// Returns: new DateTime with adjustments
DateTime DateTime_AddTimeDuration(const DateTime *dt, unsigned int hours, unsigned int minutes, unsigned int seconds);

// Returns the days in a given month (1-12) and uses leap_year parameter for February
uint8_t DateTime_DaysInMonth(uint8_t month, uint8_t leap_year);

// Returns if an year is a leap year
uint8_t DateTime_IsLeapYear(unsigned int year);

#endif // DATETIME_H
