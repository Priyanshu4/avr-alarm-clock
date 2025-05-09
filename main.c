/*
 * AlarmClock.c
 *
 * Created: 4/16/2025 1:34:50 PM
 * Author : agpri
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "uart.h"
#include "i2c_lib_S25.h"

#include "ds3231.h"
#include "lcd_dfr0555.h"
#include "datetime.h"
#include "button.h"
#include "potentiometer.h"
#include "alarmclock.h"
#include "util.h"

#define BUTTON_POLL_PERIOD_MS 20
#define POT_POLL_PERIOD_MS 20
#define DS3231_POLL_PERIOD_MS 200

#define POTENTIOMETER_AVERAGE_N_SAMPLES 10
#define POTENTIOMETER_MIN_READING 250
#define POTENTIOMETER_MAX_READING 4095

#define BUZZER_ALARM_ON_PERIOD 1000
#define BUZZER_ALARM_OFF_PERIOD 400
#define BUZZER_FREQUENCY 1500
#define BUZZER_DUTY_RATIO_ON 0.9
#define BUZZER_DUTY_RATIO_OFF 0.0

void init_clock_16_mhz() 
{
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.XOSCHFCTRLA = CLKCTRL_FRQRANGE_16M_gc | CLKCTRL_ENABLE_bm;
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_EXTCLK_gc;
}

// 1ms ISR for Timer TCA0 assuming F_CPU = 16MHz
void init_TCA0_16mhz_1ms(void)
{
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc; // Normal mode
	TCA0.SINGLE.PER = 999; // Set number of ticks for period
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm; // Enable TCA0 Overflow ISR
	// Set Prescalar to 16 & enable timer. Each tick is 4us
	TCA0.SINGLE.CTRLA |= (TCA_SINGLE_CLKSEL_DIV16_gc| TCA_SINGLE_ENABLE_bm);
}

volatile uint16_t ds3231_poll_timer_counter = 0;
volatile uint16_t button_poll_timer_counter = 0;
volatile uint16_t pot_poll_timer_counter = 0;
volatile uint16_t buzzer_timer_counter = 0;

// Executes every 1 ms.
ISR(TCA0_OVF_vect)
{
	ds3231_poll_timer_counter++;
	button_poll_timer_counter++;
	pot_poll_timer_counter++;
	buzzer_timer_counter++;
	TCA0.SINGLE.INTFLAGS |= TCA_SINGLE_OVF_bm; // must clear the interrupt
}

void init_TCA1_buzzer_pwm_pin_c4() {
	// Initialize Buzzer on C4
	PORTMUX.TCAROUTEA = PORTMUX_TCA1_PORTC_gc;
	PORTC.DIR |= PIN4_bm;
	TCA1.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc | TCA_SINGLE_ENABLE_bm;
	TCA1.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm;	
}

void set_buzzer_on_off(uint8_t buzzer_on) {
	TCA1.SINGLE.PERBUF = (F_CPU / 16) / BUZZER_FREQUENCY - 1;
	float duty_ratio = buzzer_on ? BUZZER_DUTY_RATIO_ON : BUZZER_DUTY_RATIO_OFF;
	TCA1.SINGLE.CMP0BUF = MAX(((int)(TCA1.SINGLE.PER + 1) * duty_ratio) - 1, 0);	
}


int main(void)
{
	// Initialize clock and timers
	init_clock_16_mhz();
	init_TCA0_16mhz_1ms();

	// Initialize UART (debugging)
	uart_init(3, 9600, NULL);
	
	// Initialize i2c devices (DS3231 RTC and LCD)
	ds3231_init(NULL, CLOCK_RUN, NO_FORCE_RESET);
	_delay_ms(1000);
	LCD_init();

	// Initialize buzzer
	init_TCA1_buzzer_pwm_pin_c4();
	uint8_t buzzer_on = 0;
	set_buzzer_on_off(buzzer_on);
	
	// Initialize buttons
	PORTC.DIRCLR = PIN1_bm;
	PORTC.DIRCLR = PIN2_bm;
	PORTC.DIRCLR = PIN3_bm;
	PORTC.PIN1CTRL |= PORT_PULLUPEN_bm;
	PORTC.PIN2CTRL |= PORT_PULLUPEN_bm;
	PORTC.PIN3CTRL |= PORT_PULLUPEN_bm;
	Button button1 = Button_New(&VPORTC.IN, PIN1_bm);
	Button button2 = Button_New(&VPORTC.IN, PIN2_bm);
	Button button3 = Button_New(&VPORTC.IN, PIN3_bm);
	
	// Initialize ADC to read in 12 bit mode from E0 and setup potentiometer
	ADC0.MUXPOS = ADC_MUXPOS_AIN8_gc;
	ADC0.CTRLC = ADC_PRESC_DIV16_gc;
	ADC0.CTRLD = ADC_INITDLY_DLY16_gc;
	VREF.ADC0REF = VREF_REFSEL_VDD_gc;
	ADC0.CTRLA = ADC_ENABLE_bm;
	PotentiometerReading buf[POTENTIOMETER_AVERAGE_N_SAMPLES];
	Potentiometer pot = Potentiometer_New(
		&ADC0, 
		POTENTIOMETER_MIN_READING, POTENTIOMETER_MAX_READING,
		buf, POTENTIOMETER_AVERAGE_N_SAMPLES);

	// Turn on interrupts
	sei();

	AlarmClock alarmclock = AlarmClock_Init();
	
    while (1) 
    {
		if (ds3231_poll_timer_counter >= 1000) {
			ds3231_poll_timer_counter = 0;
			AlarmClock_FetchTime(&alarmclock);
		}
		
		if (button_poll_timer_counter >= BUTTON_POLL_PERIOD_MS) {
			button_poll_timer_counter = 0;
			Button_PollingTask(&button1);
			Button_PollingTask(&button2);
			Button_PollingTask(&button3);
			
			AlarmClock_HandleButtonInput(&alarmclock, button1.state, button2.state, button3.state);
		}
		
		if (pot_poll_timer_counter >= POT_POLL_PERIOD_MS) {
			pot_poll_timer_counter = 0;
			Potentiometer_PollingTask(&pot);
			PotentiometerReading value = Potentiometer_GetAverage(&pot);	
			float pot_value = Potentiometer_ScaleValue(&pot, value, 0.0, 1.0);	
			AlarmClock_HandlePotInput(&alarmclock, pot_value);	
		}
		
		if (AlarmClock_GetBuzzerState(&alarmclock) == ALARM_CLOCK_BUZZER_BEEPING) {	
			if (buzzer_on && buzzer_timer_counter >= BUZZER_ALARM_ON_PERIOD) {
				buzzer_on = 0;
				buzzer_timer_counter = 0;
				set_buzzer_on_off(buzzer_on);
			}
			if (!buzzer_on && buzzer_timer_counter >= BUZZER_ALARM_OFF_PERIOD) {
				buzzer_on = 1;
				buzzer_timer_counter = 1;
				set_buzzer_on_off(buzzer_on);
			}
			
		} else if (buzzer_on) {
			buzzer_on = 0;
			set_buzzer_on_off(buzzer_on);	
		}
    }
}



