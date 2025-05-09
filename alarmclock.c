/*
 * alarmclock.c
 *
 * Created: 5/4/2025 7:15:42 PM
 *  Author: agpri
 */ 

#define ASSUMED_YEAR_OFFSET 2000

#include "alarmclock.h"
#include "ds3231.h"
#include "lcd_dfr0555.h"
#include "util.h"
#include <stdio.h>
#include <string.h>

// Private functions
static uint8_t update_ds3231_time(AlarmClock *clock); // Update ds3231 time to clock->current_time, return 1 if successful
static void alarm_str(Alarm* alarm, char* buf, size_t len);
static void time_display(AlarmClock *clock);
static void main_settings_display();
static void set_time_date_selection_display();
static void setting_time_display(AlarmClock *clock);
static void setting_date_display(AlarmClock *clock);
static void set_alarm_selection_display();
static void setting_alarm_display(AlarmClock *clock);
static void handle_button_input_time_display_state(AlarmClock *clock, ButtonState btn1, ButtonState btn2, ButtonState btn3);
static void handle_button_input_main_settings_state(AlarmClock *clock, ButtonState btn1, ButtonState btn2, ButtonState btn3);
static void handle_button_input_time_date_selection_state(AlarmClock *clock, ButtonState btn1, ButtonState btn2, ButtonState btn3);
static void handle_button_input_setting_time_state(AlarmClock *clock, ButtonState btn1, ButtonState btn2, ButtonState btn3);
static void handle_pot_input_setting_time_state(AlarmClock *clock, float pot_value);
static void handle_button_input_setting_date_state(AlarmClock *clock, ButtonState btn1, ButtonState btn2, ButtonState btn3);
static void handle_pot_input_setting_date_state(AlarmClock *clock, float pot_value);
static void handle_button_input_set_alarm_selection_state(AlarmClock *clock, ButtonState btn1, ButtonState btn2, ButtonState btn3);
static void handle_button_input_setting_alarm_state(AlarmClock *clock, ButtonState btn1, ButtonState btn2, ButtonState btn3);
static void handle_pot_input_setting_alarm_state(AlarmClock *clock, float pot_value);

AlarmClock AlarmClock_Init() {
	
	uint8_t time_data[7]; 
	DateTime time;
	if (ds3231_read(TIME, time_data) == OPERATION_DONE) {
		DateTime_FromDS3231Array(time_data, &time);
		print_time_data(time_data);
	}
	else {
		printf("ERROR: Failed to read initial time from DS3231\n");
		printf("WARNING: Creating AlarmClock with uninitialized time");
	}
	
	AlarmClockTimeSettingMenu time_setting_menu = {time, ALARM_CLOCK_TIME_FIELD_NONE};
	AlarmClockMenu menu = {ALARM_CLOCK_MENU_DISPLAY_TIME, time_setting_menu};
	Alarm alarm = Alarm_New(time, 0);
	AlarmClock alarmclock = {time, alarm, menu, 0};
	return alarmclock;
}

AlarmClock AlarmClock_InitWithTime(DateTime time) {
	AlarmClockTimeSettingMenu time_setting_menu = {time, ALARM_CLOCK_TIME_FIELD_NONE};
	AlarmClockMenu menu = {ALARM_CLOCK_MENU_DISPLAY_TIME, time_setting_menu};
	Alarm alarm = Alarm_New(time, 0);
	AlarmClock alarmclock = {time, alarm, menu, 0};
	return alarmclock;
}

void AlarmClock_FetchTime(AlarmClock* clock) {
			
	// Read the time from the ds3231
	uint8_t time_data[7]; 
	DateTime new_time;
	if (ds3231_read(TIME, time_data) == OPERATION_DONE) {	
		DateTime_FromDS3231Array(time_data, &new_time);
		print_time_data(time_data);
	}
	else {
		// Failed to read
		printf("ERROR: Failed to read time from DS3231\n");
		return;
	}
	
	if (DateTime_Equals(&clock->current_time, &new_time)) {
		// time is the same, no further action is needed
		return;
	}
	
	// Update time and screen
	clock->current_time = new_time;
	time_display(clock);
	
	// Check alarm trigger
	Alarm_CheckTrigger(&clock->alarm, &clock->current_time);
}

uint8_t update_ds3231_time(AlarmClock *clock) {
	uint8_t time_data[DATETIME_DS3231_DATA_LENGTH];
	DateTime_ToDS3231Array(&clock->current_time, time_data);
	if (ds3231_set(TIME, time_data) == OPERATION_DONE) {
		return 1;
	}
	else {
		printf("Error: update_ds3231_time failed!");
		return 0;
	}
}

uint8_t AlarmClock_InSettingsMenu(AlarmClock* clock) {
	return (clock->menu.state != ALARM_CLOCK_MENU_DISPLAY_TIME);
}

// Handles input from button presses
void AlarmClock_HandleButtonInput(AlarmClock* clock, ButtonState btn1, ButtonState btn2, ButtonState btn3) {
	switch (clock->menu.state) {
		case ALARM_CLOCK_MENU_DISPLAY_TIME:
			handle_button_input_time_display_state(clock, btn1, btn2, btn3);
			break;
		case ALARM_CLOCK_MENU_MAIN_SETTINGS:
			handle_button_input_main_settings_state(clock, btn1, btn2, btn3);
			break;
		case ALARM_CLOCK_MENU_SET_TIME_DATE_SELECTION:
			handle_button_input_time_date_selection_state(clock, btn1, btn2, btn3);	
			break;
		case ALARM_CLOCK_MENU_SETTING_TIME:
			handle_button_input_setting_time_state(clock, btn1, btn2, btn3);	
			break;
		case ALARM_CLOCK_MENU_SETTING_DATE:
			handle_button_input_setting_date_state(clock, btn1, btn2, btn3);
			break;
		case ALARM_CLOCK_MENU_SET_ALARM_SELECTION:
			handle_button_input_set_alarm_selection_state(clock, btn1, btn2, btn3);
			break;
		case ALARM_CLOCK_MENU_SETTING_ALARM_TIME:
			handle_button_input_setting_alarm_state(clock, btn1, btn2, btn3);
			break;
		default: break;
	}
}

void AlarmClock_HandlePotInput(AlarmClock* clock, float pot_value) {
	switch (clock->menu.state) {
		case ALARM_CLOCK_MENU_SETTING_TIME:
			handle_pot_input_setting_time_state(clock, pot_value);
			break;
		case ALARM_CLOCK_MENU_SETTING_DATE:
			handle_pot_input_setting_date_state(clock, pot_value);
			break;
		case ALARM_CLOCK_MENU_SETTING_ALARM_TIME:
			handle_pot_input_setting_alarm_state(clock, pot_value);
			break;
		default:
			break;
	}
}

BuzzerState AlarmClock_GetBuzzerState(AlarmClock *clock) {
	if (clock->alarm.state == ALARM_BEEPING) {
		return ALARM_CLOCK_BUZZER_BEEPING;
	}
	return ALARM_CLOCK_BUZZER_SILENT;
}

void alarm_str(Alarm* alarm, char* buf, size_t len) {
	if (len < 17) 
		// require 17 length (full line)
		return;
			
	switch (alarm->state) {
				
		case ALARM_DISABLED:
			strcpy(buf, "No Alarm Set");
			break;
				
		case ALARM_OFF:
		case ALARM_BEEPING:
			strcpy(buf, "Alarm ");
			DateTime_FormatTime(&alarm->time, &buf[6], len - 6, 1, 0);
			break;
				
		case ALARM_SNOOZED:
			strcpy(buf, "Snoozed ");
			DateTime_FormatTime(&alarm->snoozed_till, &buf[8], len - 8, 1, 0);
			break;
	}
}

void time_display(AlarmClock *clock) {
	if (clock->menu.state == ALARM_CLOCK_MENU_DISPLAY_TIME) {
		char line1[17];
		DateTime_FormatTime(&clock->current_time, line1, 17, 1, 1);
		LCD_printline_centered(line1, 0);
		
		char line2[17];
		
		if (clock->show_alarm_time) {
			alarm_str(&clock->alarm, line2, 17);
		}
		else if (!clock->current_time.dateValid) {
			strcpy(line2, "No Date Set");
		}
		else {
			const char* dow_str;
			char date_string[9];
			dow_str = DateTime_DayOfWeekToShortString(clock->current_time.dayOfWeek);
			DateTime_FormatDate(&clock->current_time, date_string, 9);
			sprintf(line2, "%s %s", dow_str, date_string);
		}
		
		LCD_printline_centered(line2, 1);
	}
}

void main_settings_display() {
	LCD_printline("1: Set Time/Date", 0);
	LCD_printline("2: Set Alarm", 1);
}

void set_time_date_selection_display() {
	LCD_printline("1: Set Time", 0);
	LCD_printline("2: Set Date", 1);
}

void setting_time_display(AlarmClock *clock) {
	char buf[17];
	DateTime_FormatTime(&clock->menu.time_setting.time, buf, 16, 1, 1);
	LCD_printline_centered(buf, 0);
	
	switch (clock->menu.time_setting.field) {
		case ALARM_CLOCK_TIME_FIELD_HOUR:
			LCD_printline_centered("Set Hour", 1);
			break;
		case ALARM_CLOCK_TIME_FIELD_MINUTE:
			LCD_printline_centered("Set Minute", 1);
			break;
		case ALARM_CLOCK_TIME_FIELD_SECOND:
			LCD_printline_centered("Set Second", 1);
			break;		
		case ALARM_CLOCK_TIME_FIELD_CONFIRM:
			LCD_printline_centered("Confirm Time", 1);
			break;
		
		default:
			LCD_printline_centered("Error. Restart Device.", 1);
			break;
	}
}

void setting_date_display(AlarmClock *clock) {
	char buf[17];
	const char* dow_str;
	char date_string[9];
	dow_str = DateTime_DayOfWeekToShortString(clock->menu.time_setting.time.dayOfWeek);
	DateTime_FormatDate(&clock->menu.time_setting.time, date_string, 9);
	sprintf(buf, "%s %s", dow_str, date_string);
	LCD_printline_centered(buf, 1);
	
	switch (clock->menu.time_setting.field) {
		case ALARM_CLOCK_TIME_FIELD_DAY_OF_WEEK:
			LCD_printline_centered("Set Day of Week", 0);
			break;
		case ALARM_CLOCK_TIME_FIELD_MONTH:
			LCD_printline_centered("Set Month", 0);
			break;
		case ALARM_CLOCK_TIME_FIELD_DAY:
			LCD_printline_centered("Set Day", 0);
			break;
		case ALARM_CLOCK_TIME_FIELD_YEAR:
			LCD_printline_centered("Set Year", 0);
			break;
		case ALARM_CLOCK_TIME_FIELD_CONFIRM:
			LCD_printline_centered("Confirm Date", 0);
			break;	
		
		default:
			LCD_printline_centered("Error. Restart Device.", 0);
			break;
	}
}

void set_alarm_selection_display() {
	LCD_printline("1: Set Alarm", 0);
	LCD_printline("2: Clear Alarm", 1);
}

void setting_alarm_display(AlarmClock *clock) {
	char buf[17];
	DateTime_FormatTime(&clock->menu.time_setting.time, buf, 16, 1, 1);
	LCD_printline_centered(buf, 0);
	
	switch (clock->menu.time_setting.field) {
		case ALARM_CLOCK_TIME_FIELD_HOUR:
			LCD_printline_centered("Alarm Set Hour", 1);
			break;
		case ALARM_CLOCK_TIME_FIELD_MINUTE:
			LCD_printline_centered("Alarm Set Minute", 1);
			break;
		case ALARM_CLOCK_TIME_FIELD_SECOND:
			LCD_printline_centered("Alarm Set Second", 1);
			break;
		case ALARM_CLOCK_TIME_FIELD_CONFIRM:
			LCD_printline_centered("Confirm Alarm", 1);
			break;		
		default:
			LCD_printline_centered("Error. Restart Device.", 1);
			break;
	}	
}

void handle_button_input_time_display_state(AlarmClock *clock, ButtonState btn1, ButtonState btn2, ButtonState btn3) {
	if (btn1.transition == BUTTON_JUST_PUSHED && clock->alarm.state != ALARM_BEEPING) {
		clock->menu.state = ALARM_CLOCK_MENU_MAIN_SETTINGS;
		main_settings_display();
	}
	
	if (btn2.push_state == BUTTON_PUSHED && clock->alarm.state != ALARM_BEEPING) {
		if (!clock->show_alarm_time) {
			clock->show_alarm_time = 1;
			time_display(clock);
		}
	}
	else {
		if (clock->show_alarm_time) {
			clock->show_alarm_time = 0;
			time_display(clock);
		}
	}
	
	if (btn2.transition == BUTTON_JUST_PUSHED && clock->alarm.state == ALARM_BEEPING) {
		Alarm_Off(&clock->alarm);
		time_display(clock);
	}
	
	if (btn3.transition == BUTTON_JUST_PUSHED && clock->alarm.state == ALARM_BEEPING) {
		Alarm_Snooze(&clock->alarm, &clock->current_time);
		time_display(clock);
	}	
}

void handle_button_input_main_settings_state(AlarmClock *clock, ButtonState btn1, ButtonState btn2, ButtonState btn3) {
	if (btn1.transition == BUTTON_JUST_PUSHED) {
		clock->menu.state = ALARM_CLOCK_MENU_SET_TIME_DATE_SELECTION;
		set_time_date_selection_display();
	}
	if (btn2.transition == BUTTON_JUST_PUSHED) {
		clock->menu.state = ALARM_CLOCK_MENU_SET_ALARM_SELECTION;
		set_alarm_selection_display();
	}
	if (btn3.transition == BUTTON_JUST_PUSHED) {
		clock->menu.state = ALARM_CLOCK_MENU_DISPLAY_TIME;
		time_display(clock);
	}	
}

void handle_button_input_time_date_selection_state(AlarmClock *clock, ButtonState btn1, ButtonState btn2, ButtonState btn3) {
	if (btn1.transition == BUTTON_JUST_PUSHED) {
		clock->menu.state = ALARM_CLOCK_MENU_SETTING_TIME;
		clock->menu.time_setting.time = clock->current_time;
		clock->menu.time_setting.field = ALARM_CLOCK_TIME_FIELD_HOUR;
		setting_time_display(clock);
	}
	if (btn2.transition == BUTTON_JUST_PUSHED) {
		clock->menu.state = ALARM_CLOCK_MENU_SETTING_DATE;
		clock->menu.time_setting.time = clock->current_time;
		clock->menu.time_setting.field = ALARM_CLOCK_TIME_FIELD_DAY_OF_WEEK;
		setting_date_display(clock);
	}
	if (btn3.transition == BUTTON_JUST_PUSHED) {
		clock->menu.state = ALARM_CLOCK_MENU_MAIN_SETTINGS;
		main_settings_display();
	}	
}

void handle_button_input_setting_time_state(AlarmClock *clock, ButtonState btn1, ButtonState btn2, ButtonState btn3) {
	if (btn2.transition == BUTTON_JUST_PUSHED) {
		switch (clock->menu.time_setting.field) {
			case ALARM_CLOCK_TIME_FIELD_HOUR:
			case ALARM_CLOCK_TIME_FIELD_MINUTE:
				clock->menu.time_setting.field = clock->menu.time_setting.field + 1;
				setting_time_display(clock);
				break;
				
			case ALARM_CLOCK_TIME_FIELD_SECOND:
				clock->menu.time_setting.field = ALARM_CLOCK_TIME_FIELD_CONFIRM;
				setting_time_display(clock);
				break;
							
			case ALARM_CLOCK_TIME_FIELD_CONFIRM:
				clock->current_time = clock->menu.time_setting.time;
				update_ds3231_time(clock);
				clock->menu.state = ALARM_CLOCK_MENU_DISPLAY_TIME;
				time_display(clock);
				break;
	
			default:
				printf("ERROR! Invalid time field state reached\n");
				break;
		}
	}
	if (btn3.transition == BUTTON_JUST_PUSHED) {
		switch (clock->menu.time_setting.field) {
			
			case ALARM_CLOCK_TIME_FIELD_HOUR:
				clock->menu.state = ALARM_CLOCK_MENU_SET_TIME_DATE_SELECTION;
				set_time_date_selection_display();
				break;
			
			case ALARM_CLOCK_TIME_FIELD_MINUTE:
			case ALARM_CLOCK_TIME_FIELD_SECOND:
				clock->menu.time_setting.field = clock->menu.time_setting.field - 1;
				setting_time_display(clock);
				break;
					
			case ALARM_CLOCK_TIME_FIELD_CONFIRM:
				clock->menu.state = ALARM_CLOCK_TIME_FIELD_SECOND;
				setting_time_display(clock);
				break;
							
			default:
				printf("ERROR! Invalid time field state reached\n");
				break;
		}
	}
}

void handle_pot_input_setting_time_state(AlarmClock *clock, float pot_value) {
	switch (clock->menu.time_setting.field) {
		case ALARM_CLOCK_TIME_FIELD_HOUR:
			clock->menu.time_setting.time.hour = MIN((uint8_t) ScaleFloat(pot_value, 0, 1, 0, 24), 23);
			setting_time_display(clock);
			break;
		case ALARM_CLOCK_TIME_FIELD_MINUTE:
			clock->menu.time_setting.time.minute = MIN((uint8_t) ScaleFloat(pot_value, 0, 1, 0, 60), 59);
			setting_time_display(clock);
			break;
		case ALARM_CLOCK_TIME_FIELD_SECOND:
			clock->menu.time_setting.time.second = MIN((uint8_t) ScaleFloat(pot_value, 0, 1, 0, 60), 59);
			setting_time_display(clock);
			break;
		default:
			break;
	}	
}

void handle_button_input_setting_date_state(AlarmClock *clock, ButtonState btn1, ButtonState btn2, ButtonState btn3) {
	if (btn2.transition == BUTTON_JUST_PUSHED) {
		switch (clock->menu.time_setting.field) {
			case ALARM_CLOCK_TIME_FIELD_DAY_OF_WEEK:
			case ALARM_CLOCK_TIME_FIELD_MONTH:
			case ALARM_CLOCK_TIME_FIELD_DAY:
				clock->menu.time_setting.field = clock->menu.time_setting.field + 1;
				setting_date_display(clock);
				break;

			case ALARM_CLOCK_TIME_FIELD_YEAR:
				clock->menu.time_setting.field = ALARM_CLOCK_TIME_FIELD_CONFIRM;
				setting_date_display(clock);
				break;

			case ALARM_CLOCK_TIME_FIELD_CONFIRM:
				clock->current_time = clock->menu.time_setting.time;
				update_ds3231_time(clock);
				clock->menu.state = ALARM_CLOCK_MENU_DISPLAY_TIME;
				time_display(clock);
				break;

			default:
				printf("ERROR! Invalid date field state reached\n");
				break;
		}
	}

	if (btn3.transition == BUTTON_JUST_PUSHED) {
		switch (clock->menu.time_setting.field) {
			case ALARM_CLOCK_TIME_FIELD_DAY_OF_WEEK:
				clock->menu.state = ALARM_CLOCK_MENU_SET_TIME_DATE_SELECTION;
				set_time_date_selection_display();
				break;

			case ALARM_CLOCK_TIME_FIELD_MONTH:
			case ALARM_CLOCK_TIME_FIELD_DAY:
			case ALARM_CLOCK_TIME_FIELD_YEAR:
				clock->menu.time_setting.field = clock->menu.time_setting.field - 1;
				setting_date_display(clock);
				break;

			case ALARM_CLOCK_TIME_FIELD_CONFIRM:
				clock->menu.time_setting.field = ALARM_CLOCK_TIME_FIELD_YEAR;
				setting_date_display(clock);
				break;

			default:
				printf("ERROR! Invalid date field state reached\n");
				break;
		}
	}
}

void handle_pot_input_setting_date_state(AlarmClock *clock, float pot_value) {
	switch (clock->menu.time_setting.field) {
		case ALARM_CLOCK_TIME_FIELD_DAY_OF_WEEK:
			clock->menu.time_setting.time.dayOfWeek = MIN((uint8_t)ScaleFloat(pot_value, 0, 1, 1, 8), 7);
			setting_date_display(clock);
			break;
		case ALARM_CLOCK_TIME_FIELD_MONTH:
			clock->menu.time_setting.time.month = MIN((uint8_t)ScaleFloat(pot_value, 0, 1, 1, 13), 12);
			setting_date_display(clock);
			break;
		case ALARM_CLOCK_TIME_FIELD_DAY: {
			uint8_t max_day = DateTime_DaysInMonth(clock->menu.time_setting.time.month, DateTime_IsLeapYear(clock->menu.time_setting.time.year + ASSUMED_YEAR_OFFSET));
			clock->menu.time_setting.time.day = MIN((uint8_t)ScaleFloat(pot_value, 0, 1, 1, max_day+1), max_day);
			setting_date_display(clock);
			break;
		}
		case ALARM_CLOCK_TIME_FIELD_YEAR:
			// Stored as two-digit year (00–99)
			clock->menu.time_setting.time.year = MIN((uint8_t)ScaleFloat(pot_value, 0, 1, 0, 100), 99);
			setting_date_display(clock);
			break;
		default:
			break;
	}
}

void handle_button_input_set_alarm_selection_state(AlarmClock *clock, ButtonState btn1, ButtonState btn2, ButtonState btn3) {
	if (btn1.transition == BUTTON_JUST_PUSHED) {
		// setting alarm
		clock->menu.state = ALARM_CLOCK_MENU_SETTING_ALARM_TIME;
		clock->menu.time_setting.time = clock->alarm.time;
		clock->menu.time_setting.field = ALARM_CLOCK_TIME_FIELD_HOUR;
		setting_alarm_display(clock);
	}
	if (btn2.transition == BUTTON_JUST_PUSHED) {
		// alarm cleared
		clock->menu.state = ALARM_CLOCK_MENU_DISPLAY_TIME;
		clock->alarm.state = ALARM_DISABLED;
		time_display(clock);
	}
	if (btn3.transition == BUTTON_JUST_PUSHED) {
		// back to main settings page
		clock->menu.state = ALARM_CLOCK_MENU_MAIN_SETTINGS;
		main_settings_display();
	}	
}

void handle_button_input_setting_alarm_state(AlarmClock *clock, ButtonState btn1, ButtonState btn2, ButtonState btn3) {
	if (btn2.transition == BUTTON_JUST_PUSHED) {
		switch (clock->menu.time_setting.field) {
			case ALARM_CLOCK_TIME_FIELD_HOUR:
			case ALARM_CLOCK_TIME_FIELD_MINUTE:
				// advance to next field
				clock->menu.time_setting.field = clock->menu.time_setting.field + 1;
				setting_alarm_display(clock);
				break;

			case ALARM_CLOCK_TIME_FIELD_SECOND:
				// move to confirmation
				clock->menu.time_setting.field = ALARM_CLOCK_TIME_FIELD_CONFIRM;
				setting_alarm_display(clock);
				break;

			case ALARM_CLOCK_TIME_FIELD_CONFIRM:
				// apply the new alarm time and enable it
				clock->alarm.time  = clock->menu.time_setting.time;
				clock->alarm.state = ALARM_OFF;
				// return to normal display
				clock->menu.state = ALARM_CLOCK_MENU_DISPLAY_TIME;
				time_display(clock);
				break;

			default:
				printf("ERROR! Invalid alarm field state reached\n");
				break;
		}
	}

	if (btn3.transition == BUTTON_JUST_PUSHED) {
		switch (clock->menu.time_setting.field) {
			case ALARM_CLOCK_TIME_FIELD_HOUR:
				// back to alarm menu
				clock->menu.state = ALARM_CLOCK_MENU_SET_ALARM_SELECTION;
				set_alarm_selection_display();
				break;

			case ALARM_CLOCK_TIME_FIELD_MINUTE:
			case ALARM_CLOCK_TIME_FIELD_SECOND:
				// move back one field
				clock->menu.time_setting.field = clock->menu.time_setting.field - 1;
				setting_alarm_display(clock);
				break;

			case ALARM_CLOCK_TIME_FIELD_CONFIRM:
				// undo confirm, go back to seconds
				clock->menu.time_setting.field = ALARM_CLOCK_TIME_FIELD_SECOND;
				setting_alarm_display(clock);
				break;

			default:
				printf("ERROR! Invalid alarm field state reached\n");
				break;
		}
	}
}

void handle_pot_input_setting_alarm_state(AlarmClock *clock, float pot_value) {
	switch (clock->menu.time_setting.field) {
		case ALARM_CLOCK_TIME_FIELD_HOUR:
			clock->menu.time_setting.time.hour   = MIN((uint8_t)ScaleFloat(pot_value, 0, 1, 0, 24), 23);
			setting_alarm_display(clock);
			break;

		case ALARM_CLOCK_TIME_FIELD_MINUTE:
			clock->menu.time_setting.time.minute = MIN((uint8_t)ScaleFloat(pot_value, 0, 1, 0, 60), 59);
			setting_alarm_display(clock);
			break;

		case ALARM_CLOCK_TIME_FIELD_SECOND:
			clock->menu.time_setting.time.second = MIN((uint8_t)ScaleFloat(pot_value, 0, 1, 0, 60), 59);
			setting_alarm_display(clock);
			break;

		default:
			break;
	}
}

