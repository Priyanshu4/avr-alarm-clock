/*
 * potentiometer.h
 *
 * Created: 4/23/2025 2:28:16 PM
 *  Author: agpri
 */ 

#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H

#include <avr/io.h>
#include <stdint.h>

typedef int16_t PotentiometerReading;

typedef struct {
	ADC_t* adc;
	PotentiometerReading value;
	
	// The minimum and maximum value that can be read from the ADC
	// in practice, the pot doesn't reach 0 volt or max voltage even when turned all the way, so we need this
	PotentiometerReading min;
	PotentiometerReading max;

	// Ring buffer that stores the last buffer_size readings for averaging
	PotentiometerReading* buffer;
	uint8_t buffer_size;
	uint8_t buffer_index;
	uint8_t buffer_count;
} Potentiometer;

/**
 * Initializes a Potentiometer.
 * Arguments:
 * - `adc` is the ADC peripheral already configured.
 * - `minimum_reading` is the minimum value that is ever read by the ADC
 * - `maximum_reading` is the maximum value that is ever read by the ADC
 * - `buffer` is a user-allocated array of length `buffer_size`. The last buffer size readings are averaged by GetAverage();
 *
 * Returns the Potentiometer object.
 */
Potentiometer Potentiometer_New(ADC_t* adc, PotentiometerReading minimum_reading, PotentiometerReading max_reading, PotentiometerReading * buffer, uint8_t buffer_size);

// Reads the potentiometer, updates `pot->value` and adds to the averaging buffer.
void Potentiometer_PollingTask(Potentiometer *pot);

// Returns the average of the recent buffered values.
PotentiometerReading Potentiometer_GetAverage(Potentiometer *pot);

// Scales a raw potentiometer value to a desired float range.
float Potentiometer_ScaleValue(Potentiometer *pot, PotentiometerReading value, float min, float max);

#endif // POTENTIOMETER_H
