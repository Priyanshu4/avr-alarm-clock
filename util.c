/*
 * util.c
 *
 * Created: 5/2/2025 11:24:20 PM
 *  Author: agpri
 */ 

#include "util.h"

float ScaleFloat(float value, float in_min, float in_max, float out_min, float out_max) {
	if (in_max == in_min) {
		// Avoid divide-by-zero; return center of output range
		return (out_min + out_max) * 0.5f;
	}

	float normalized = (value - in_min) / (in_max - in_min);
	return normalized * (out_max - out_min) + out_min;
}
