/*
 * potentiometer.c
 *
 * Created: 4/23/2025 2:39:42 PM
 *  Author: agpri
 */ 

#include "potentiometer.h"
#include "util.h"
#include <avr/sfr_defs.h>

Potentiometer Potentiometer_New(
	ADC_t* adc, 
	PotentiometerReading minimum_reading, 
	PotentiometerReading maximum_reading, 
	PotentiometerReading* buffer, 
	uint8_t buffer_size) 
{
	Potentiometer pot = {
		.adc = adc,
		.value = 0,
		.min = minimum_reading,
		.max = maximum_reading,
		.buffer = buffer,
		.buffer_size = buffer_size,
		.buffer_index = 0,
		.buffer_count = 0
	};
	return pot;
}

void Potentiometer_PollingTask(Potentiometer *pot) {
	ADC_t* ADC = pot->adc;

	// Start ADC conversion
	ADC->COMMAND = ADC_STCONV_bm;

	// Wait for conversion to finish
	loop_until_bit_is_clear(ADC->COMMAND, ADC_STCONV_bp);

	// Read value
	PotentiometerReading new_value = ADC->RES;
	pot->value = new_value;

	// Store in ring buffer
	if (pot->buffer && pot->buffer_size > 0) {
		pot->buffer[pot->buffer_index] = new_value;
		pot->buffer_index = (pot->buffer_index + 1) % pot->buffer_size;
		if (pot->buffer_count < pot->buffer_size) {
			pot->buffer_count++;
		}
	}
}

PotentiometerReading Potentiometer_GetAverage(Potentiometer *pot) {
	if (!pot->buffer || pot->buffer_count == 0) {
		// if buffer is null or 0 size then just return the latest reading
		return pot->value;  
	}

	int32_t sum = 0;
	for (uint8_t i = 0; i < pot->buffer_count; i++) {
		sum += pot->buffer[i];
	}
	return (PotentiometerReading)(sum / pot->buffer_count);
}

float Potentiometer_ScaleValue(Potentiometer* pot, PotentiometerReading value, float min, float max) {
	float normalized = ((float)(value - pot->min)) / ((float)(pot->max - pot->min));
	if (normalized < 0.0f) normalized = 0.0f;
	if (normalized > 1.0f) normalized = 1.0f;
	return normalized * (max - min) + min;
}
