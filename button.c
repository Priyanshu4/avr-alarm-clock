/*
 * button_sm.c
 *
 * Created: 4/14/2025 3:02:00 PM
 *  Author: agpri
 */ 
#include "button.h"

Button Button_New(volatile uint8_t* input_register, uint8_t bitmask) {
	ButtonState state = {
		BUTTON_RELEASED,
		BUTTON_NO_TRANSITION
	};
	
	Button button = {
		input_register,
		bitmask,
		state
	};
	
	return button;
}

void Button_PollingTask(Button *button) {
	uint8_t register_value = *(button->input_register);
	ButtonState* state_ptr = &(button->state);
	Button_StateMachine( 
		!(register_value & button->bitmask),
		state_ptr
	);
}

void Button_StateMachine(int buttonPressed, ButtonState *state) {
	state->transition = BUTTON_NO_TRANSITION;

	switch (state->push_state) {
		case BUTTON_RELEASED:
			if (buttonPressed) {
				state->push_state = BUTTON_MAYBE_PUSHED;
			}
		break;
		
		case BUTTON_MAYBE_PUSHED:
			if (buttonPressed) {
				state->push_state = BUTTON_PUSHED;
				state->transition = BUTTON_JUST_PUSHED;
			}
			else {
				state->push_state = BUTTON_RELEASED;
			}
		break;
		
		case BUTTON_PUSHED:
			if (!buttonPressed) {
				state->push_state = BUTTON_MAYBE_RELEASED;	
			}
		break;
		
		case BUTTON_MAYBE_RELEASED:
			if (buttonPressed) {
				state->push_state = BUTTON_PUSHED;
			}
			else {
				state->push_state = BUTTON_RELEASED;
				state->transition = BUTTON_JUST_RELEASED;
			}
		break;
	}
}
