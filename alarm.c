/*
 * alarm.c
 *
 * Created: 4/28/2025 3:38:54 PM
 *  Author: agpri
 */ 

#include "alarm.h"

Alarm Alarm_New(DateTime alarm_time, uint8_t enabled) {
	AlarmState state = enabled ? ALARM_OFF : ALARM_DISABLED;
	Alarm alarm = {alarm_time, state, alarm_time};
	return alarm;
}

uint8_t Alarm_CheckTrigger(Alarm *alarm, const DateTime *current_time) {	
	if (alarm->state == ALARM_OFF && DateTime_TimeEquals(&alarm->time, current_time))
	{
		alarm->state = ALARM_BEEPING;	
		return 1;
	}
	if (alarm->state == ALARM_SNOOZED && DateTime_TimeEquals(&alarm->snoozed_till, current_time))
	{
		alarm->state = ALARM_BEEPING;
		return 1;
	}
	return 0;
}

// Snooze the alarm
void Alarm_Snooze(Alarm *alarm, const DateTime *current_time) {
	if (alarm->state == ALARM_BEEPING) {
		alarm->state = ALARM_SNOOZED;
		alarm->snoozed_till = DateTime_AddTimeDuration(current_time, 0, ALARM_SNOOZE_MINUTES, 0);
	}
}

// Snooze the alarm
void Alarm_Off(Alarm *alarm) {
	if (alarm->state == ALARM_BEEPING) {
		alarm->state = ALARM_OFF;
	}
}
