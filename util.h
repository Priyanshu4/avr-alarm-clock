/*
 * util.h
 *
 * Created: 4/30/2025 2:17:13 PM
 *  Author: agpri
 */ 

#ifndef ALARM_CLOCK_UTIL_H
#define ALARM_CLOCK_UTIL_H

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

/**
 * Linearly maps a float value from one range to another.
 * If in_max == in_min, returns the midpoint of the output range.
 *
 * Arguments
 * - value     The input value to scale
 * - in_min    The lower bound of the input range
 * - in_max    The upper bound of the input range
 * - out_min   The lower bound of the output range
 * - out_max   The upper bound of the output range
 * 
 * Returns the scaled float value
 */
float ScaleFloat(float value, float in_min, float in_max, float out_min, float out_max);

#endif // ALARM_CLOCK_UTIL_H