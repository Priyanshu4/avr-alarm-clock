/*
 * alarm.h
 *
 * Created: 4/28/2025 3:39:08 PM
 *  Author: agpri
 */ 

#ifndef ALARM_H
#define ALARM_H

#include "datetime.h"

#define ALARM_SNOOZE_MINUTES 10

typedef enum {
	ALARM_OFF,
	ALARM_BEEPING,
	ALARM_SNOOZED,
	ALARM_DISABLED,
} AlarmState;

typedef struct {
	DateTime time; // only the time part is relevant, not the date
	AlarmState state;
	DateTime snoozed_till;	// only the time part is relevant, not the date
} Alarm;

// Create a new alarm
Alarm Alarm_New(DateTime alarm_time, uint8_t enabled);

// Check if alarm should be triggered
uint8_t Alarm_CheckTrigger(Alarm *alarm, const DateTime* current_time);

// Snooze the alarm
void Alarm_Snooze(Alarm *alarm, const DateTime* current_time);

// Turn off the alarm till next day
void Alarm_Off(Alarm *alarm);

#endif // ALARM_H
