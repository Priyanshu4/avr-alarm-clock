/*
 * alarmclock.h
 *
 * Created: 5/03/2025 3:17:03 PM
 *  Author: agpri
 */

#ifndef ALARM_CLOCK_H
#define ALARM_CLOCK_H

#include "datetime.h"
#include "alarm.h"
#include "button.h"
#include "potentiometer.h"

typedef enum {
	ALARM_CLOCK_TIME_FIELD_DAY_OF_WEEK,
	ALARM_CLOCK_TIME_FIELD_MONTH,
	ALARM_CLOCK_TIME_FIELD_DAY,
	ALARM_CLOCK_TIME_FIELD_YEAR,
	ALARM_CLOCK_TIME_FIELD_HOUR,
	ALARM_CLOCK_TIME_FIELD_MINUTE,
	ALARM_CLOCK_TIME_FIELD_SECOND,
	ALARM_CLOCK_TIME_FIELD_CONFIRM,
	ALARM_CLOCK_TIME_FIELD_NONE,
} TimeField;

typedef struct {
	DateTime time;
	TimeField field;
} AlarmClockTimeSettingMenu;

typedef enum {
	ALARM_CLOCK_MENU_DISPLAY_TIME,
	ALARM_CLOCK_MENU_MAIN_SETTINGS,
	ALARM_CLOCK_MENU_SET_TIME_DATE_SELECTION,
	ALARM_CLOCK_MENU_SETTING_TIME,
	ALARM_CLOCK_MENU_SETTING_DATE,
	ALARM_CLOCK_MENU_SET_ALARM_SELECTION,
	ALARM_CLOCK_MENU_SETTING_ALARM_TIME,
} AlarmClockMenuState;

typedef struct {
	AlarmClockMenuState state;
	AlarmClockTimeSettingMenu time_setting;
} AlarmClockMenu;

typedef struct {
	DateTime current_time;
	Alarm alarm;
	AlarmClockMenu menu;
	uint8_t show_alarm_time; // if 1, show the alarm time instead of the weekday month/day/year, controlled by a button
} AlarmClock;

// Initializes and returns the AlarmClock in the initial state.
AlarmClock AlarmClock_Init();

// Initializes and returns the AlarmClock in the initial state and time
AlarmClock AlarmClock_InitWithTime(DateTime time);

// The clock reads the updated time from the DS3231
void AlarmClock_FetchTime(AlarmClock* clock);

// Returns 1 if the alarm clock is in the settings menu, 0 otherwise
uint8_t AlarmClock_InSettingsMenu(AlarmClock* clock);

// Handles input from button presses
void AlarmClock_HandleButtonInput(AlarmClock* clock, ButtonState btn1, ButtonState btn2, ButtonState btn3);

/*
 * Handle input from potentiometer turns
 * Arguments:
 * - clock: ptr to AlarmClock object
 * - pot_value: potentiometer value, between 0 to 1
 */
void AlarmClock_HandlePotInput(AlarmClock* clock, float pot_value);

typedef enum {
	ALARM_CLOCK_BUZZER_BEEPING,
	ALARM_CLOCK_BUZZER_SILENT,
} BuzzerState;

// Return the state of the AlarmClock's buzzer
BuzzerState AlarmClock_GetBuzzerState(AlarmClock *clock);

#endif // ALARM_CLOCK_H