
/*
 * button_sm.h
 *
 * Created: 4/14/2025 3:01:00 PM
 *  Author: agpri
 */ 

#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>

typedef enum {
	BUTTON_RELEASED,
	BUTTON_MAYBE_PUSHED,
	BUTTON_PUSHED,
	BUTTON_MAYBE_RELEASED
} ButtonPushState;

typedef enum {
	BUTTON_JUST_PUSHED,
	BUTTON_JUST_RELEASED,
	BUTTON_NO_TRANSITION,
} ButtonStateTransition;

typedef struct {
	ButtonPushState push_state;
	ButtonStateTransition transition; 
} ButtonState;

typedef struct {
	volatile uint8_t* input_register;
	uint8_t bitmask;
	ButtonState state;
} Button;

// Creates a new button object. Buttons should be pull-up.
Button Button_New(volatile uint8_t* input_register, uint8_t bitmask);

// Reads the input register and updates button state. Should be invoked periodically.
void Button_PollingTask(Button *button);

// Progresses the button state based on if its pressed.
void Button_StateMachine(int buttonPressed, ButtonState *state);

#endif // BUTTON_H