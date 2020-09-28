/* Library declarations */
#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 8000000UL // Internal clock frequency definition
#include <util/delay.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include "rtc.h"
#include "lc.h"
#include "Display.h"

/* Global in-used variables */
int i,j = 0x31;				// Future-use. 
int string_requested = 0;	// State purpose.
uint8_t out_confirm = 0;	// Menu state Machine purposes.
uint8_t buzzer_state = buzzer_on;	// Initial state of the buzzer on/off select
char buf[2];		// Integer to ASCII function 2-characters array 

/* Alarm time set values */
uint8_t sec_alarm =  100;
uint8_t min_alarm =  100;
uint8_t hour_alarm = 100;

/* Complete peripheral devices initialization */ 
void Init_Device(void)
{
	DDRD |= (1 << 0); // Buzzer output initialization
	Init_LED_MATRIX();
	Init_Display();
	Init_RGB_LED();
	Init_switches();
}

/* Smart greetings screen - according to the hour value, provided by RTC */
void greetings(uint8_t hour_in)
{
	open_matrix_SPI();
	print_string("  GOOD  ");
	close_matrix_SPI();
	_delay_ms(1000);
	open_matrix_SPI();
	if (hour_in >= 0 && hour_in < 4)	print_string(" NIGHT! ");
	if (hour_in >= 4 && hour_in < 12)	print_string("MORNING!");
	if (hour_in >= 12 && hour_in < 17)	print_string("  DAY!  ");
	if (hour_in >= 17 && hour_in <= 23) print_string("EVENING!");
	else ;
	close_matrix_SPI();
	_delay_ms(1000);
}

/* Integer to ASCII array conversion, only two digits are used, since there is no need for more */
void int_to_ascii(int xxx)
{
	if (xxx <= 9)
	{
		buf[0] = 0x30;
		buf[1] = xxx + 0x30;
	}
	else
	{
	buf[1] = xxx % 10 + 0x30;
	xxx /= 10;
	buf[0] = xxx % 10 + 0x30;
	}
}
	
/* Real-time clock string generation */
void print_time_string(int second_in, int minute_in, int hour_in)
{
	int_to_ascii(hour_in);
	Print_Character(buf[0]);
	Print_Character(buf[1]);
	Print_Character(':');
	int_to_ascii(minute_in);
	Print_Character(buf[0]);
	Print_Character(buf[1]);
	Print_Character(':');
	int_to_ascii(second_in);
	Print_Character(buf[0]);
	Print_Character(buf[1]);	
}

/* Date: integer value to month name string conversion */ 
void month_string_determine (uint8_t month_in)
{
	switch(month_in)
	{
		case 1: print_string("JAN"); break;
		case 2: print_string("FEB"); break;
		case 3: print_string("MAR"); break;
		case 4: print_string("APR"); break;
		case 5: print_string("MAY"); break;
		case 6: print_string("JUN"); break;
		case 7: print_string("JUL"); break;
		case 8: print_string("AUG"); break;
		case 9: print_string("SEP"); break;
		case 10:print_string("OCT"); break;
		case 11:print_string("NOV"); break;
		case 12:print_string("DEC"); break;
		default: break;
	}
}

/* Date string display */
void print_date_string(int day_in, int month_in)
{
	Print_Character('|');
	int_to_ascii(day_in);
	Print_Character(buf[0]);
	Print_Character(buf[1]);
	Print_Character('/');
	month_string_determine(month_in);
	Print_Character('|');
}

/* User-input Push Button initialization */
void Init_switches(void)
{
	DDRD &= ~(1 << SW_MODE); 
	DDRD &= ~(1 << SW_ADJUST);
	DDRD &= ~(1 << SW_RGB);
}

/* RGB LED outputs initialization */
void Init_RGB_LED(void)
{
	DDRD |= (1 << RED) | (1 << GREEN);
	DDRB |= (1 << BLUE);
	TCCR1A |= (1 << COM1A1) | (1 << WGM10);
	TCCR1B |= (1 << CS10) | (1 << WGM12);	
	TCCR0A |= _BV(COM0A1) | _BV(WGM00) | _BV(WGM01) |_BV(COM0B1) ;
	TCCR0B |= _BV(CS00);
	
}

/* Stop all the running timers (PWM mode RGB disable) */
void stop_timers(void)
{
	TCCR1A &= ~(1 << COM1A1) & ~(1 << WGM10);
	TCCR1B &= ~(1 << CS10) & ~(1 << WGM12);
	TCCR0A &= ~(1 << COM0A1) & ~(1 << WGM00) & ~(1 << WGM01) & ~(1 << COM0B1) ;
	TCCR0B &= ~(1 << CS00);
	
}

/* PWM output on the RGB LED enable */
void PWM_OUT(int color_type, int duty_cycle)
{
	if (color_type == GREEN)	OCR0A = duty_cycle;
	if (color_type == BLUE)		OCR1A = duty_cycle;
	if (color_type == RED)		OCR0B = duty_cycle;
}

/* PWM Generating function for RGB LED */
void showRGB(int color)
{
	int redIntensity;
	int greenIntensity;
	int blueIntensity;

	// Here we'll use an "if / else" statement to determine which
	// of the three (R,G,B) zones x falls into. Each of these zones
	// spans 255 because analogWrite() wants a number from 0 to 255.

	// In each of these zones, we'll calculate the brightness
	// for each of the red, green, and blue LEDs within the RGB LED.

	if (color <= 255)          // zone 1
	{
		redIntensity = 255 - color;    // red goes from on to off
		greenIntensity = color;        // green goes from off to on
		blueIntensity = 0;             // blue is always off
	}
	else if (color <= 511)     // zone 2
	{
		redIntensity = 0;                     // red is always off
		greenIntensity = 255 - (color - 256); // green on to off
		blueIntensity = (color - 256);        // blue off to on
	}
	else // color >= 512       // zone 3
	{
		redIntensity = (color - 512);         // red off to on
		greenIntensity = 0;                   // green is always off
		blueIntensity = 255 - (color - 512);  // blue on to off
	}

	// Now that the brightness values have been set, command the LED
	// to those values

	PWM_OUT(RED, redIntensity);
	PWM_OUT(BLUE, blueIntensity);
	PWM_OUT(GREEN, greenIntensity);
}

/* Push button pressed check */
uint8_t poll_switch (void)
{
	uint8_t ret_int = 0;
	switch(PIND & ( (1 << SW_MODE) | (1 << SW_ADJUST) | (1 << SW_RGB) ))
	{
		case 0b00001100: 
			ret_int = SW_MODE;
		break;
		case 0b00001010:  
			ret_int = SW_RGB;
		break;
		case 0b00000110: 
			ret_int = SW_ADJUST;
		break;
		default: break;
	}
	
	return ret_int;
}

/* Main menu switch between states */
uint8_t ladder_string(uint8_t current_state, uint8_t operator_in)
{
	uint8_t retvalue = 0;
	switch(current_state)
	{
		case MENU_ADJUST:
			if (operator_in == up) 
			{
			retvalue = MENU_COLORS;
			strcpy(menu_string, COLORS);
			}
			if (operator_in == down)
			{
				retvalue = MENU_EXIT;
				strcpy(menu_string, EXIT_MENU);
			}
		break;
		
		case MENU_COLORS:
			if (operator_in == up)
			{
				retvalue = MENU_ALARM;
				strcpy(menu_string, ALARM);
			}
			if (operator_in == down)
			{
				retvalue = MENU_ADJUST;
				strcpy(menu_string, ADJUST);
			}
			break;

		case MENU_ALARM:
			if (operator_in == up)
			{
				retvalue = MENU_DISPLAY;
				strcpy(menu_string, DISPLAY);
			}
			if (operator_in == down)
			{
				retvalue = MENU_COLORS;
				strcpy(menu_string, COLORS);
			}
			break;		

		case MENU_DISPLAY:
			if (operator_in == up)
			{
				retvalue = MENU_BUZZER;
				strcpy(menu_string, BUZZER);
			}
			if (operator_in == down)
			{
				retvalue = MENU_ALARM;
				strcpy(menu_string, ALARM);
			}
			break;

		case MENU_BUZZER:
			if (operator_in == up)
			{
				retvalue = MENU_EXIT;
				strcpy(menu_string, EXIT_MENU);
			}
			if (operator_in == down)
			{
				retvalue = MENU_DISPLAY;
				strcpy(menu_string, DISPLAY);
			}
			break;		

		case MENU_EXIT:
			if (operator_in == up)
			{
				retvalue = MENU_ADJUST;
				strcpy(menu_string, ADJUST);
			}
			if (operator_in == down)
			{
				retvalue = MENU_BUZZER;
				strcpy(menu_string, BUZZER);
			}
			break;
			default: break;
	}
return retvalue;
}

/* Color menu switch between states */
uint8_t ladder_string_color(uint8_t current_state, uint8_t operator_in)
{
	uint8_t retvalue = 0;
	switch(current_state)
	{
		case R_mode:
		if (operator_in == up)
		{
			retvalue = G_mode;
			strcpy(menu_string, GREENST);
		}
		if (operator_in == down)
		{
			retvalue = off_mode;
			strcpy(menu_string, OFFST);
		}
		break;
		
		case G_mode:
		if (operator_in == up)
		{
			retvalue = B_mode;
			strcpy(menu_string, BLUEST);
		}
		if (operator_in == down)
		{
			retvalue = R_mode;
			strcpy(menu_string, REDST);
		}
		break;

		case B_mode:
		if (operator_in == up)
		{
			retvalue = PWM_mode;
			strcpy(menu_string, PWMST);
		}
		if (operator_in == down)
		{
			retvalue = G_mode;
			strcpy(menu_string, GREENST);
		}
		break;

		case PWM_mode:
		if (operator_in == up)
		{
			retvalue = off_mode;
			strcpy(menu_string, OFFST);
		}
		if (operator_in == down)
		{
			retvalue = B_mode;
			strcpy(menu_string, BLUEST);
		}
		break;

		case off_mode:
		if (operator_in == up)
		{
			retvalue = R_mode;
			strcpy(menu_string, REDST);
		}
		if (operator_in == down)
		{
			retvalue = PWM_mode;
			strcpy(menu_string, PWMST);
		}
		break;

	}
	return retvalue;
}

/* Display menu switch between states */
uint8_t ladder_string_display(uint8_t current_state, uint8_t operator_in)
{
	uint8_t retvalue = 0;
	switch(current_state)
	{
		case time_mode:
		if (operator_in == up)
		{
			retvalue = date_mode;
			strcpy(menu_string, DATEST);
		}
		if (operator_in == down)
		{
			retvalue = both_mode;
			strcpy(menu_string, BOTHST);
		}
		break;
		
		case date_mode:
		if (operator_in == up)
		{
			retvalue = both_mode;
			strcpy(menu_string, BOTHST);
		}
		if (operator_in == down)
		{
			retvalue = time_mode;
			strcpy(menu_string, TIMEST);
		}
		break;

		case both_mode:
		if (operator_in == up)
		{
			retvalue = time_mode;
			strcpy(menu_string, TIMEST);
		}
		if (operator_in == down)
		{
			retvalue = date_mode;
			strcpy(menu_string, DATEST);
		}
		break;
	}
	return retvalue;
}

/* Main menu modes function */
uint8_t menu_mode (void)
{
	uint8_t current_state = MENU_ADJUST;
	strcpy(menu_string,ADJUST); 
	uint8_t out_confirm_x = 0;
			while(out_confirm_x == 0)
			{
				open_matrix_SPI();
				print_string(menu_string);
				close_matrix_SPI();
				if (poll_switch() == SW_MODE) 
				{
					beep(buzzer_state);
					while(poll_switch() == SW_MODE);
					out_confirm_x = 1;
				}
				else if (poll_switch() == SW_ADJUST) 
				{
					beep(buzzer_state);
					while(poll_switch() == SW_ADJUST);
					current_state = ladder_string(current_state, up);
				}
				else if (poll_switch() == SW_RGB)
				{
					beep(buzzer_state);
					while(poll_switch() == SW_RGB);
					current_state = ladder_string(current_state, down);
				}
			}
return current_state;
}

/* Display mode menu function */
uint8_t menu_mode_display(void)
{
//	int rcnt = 0;
	uint8_t current_state = time_mode;
//	uint8_t ret_val = 0;
	strcpy(menu_string,TIMEST);
	uint8_t out_confirm_x = 0;
	while(out_confirm_x == 0)
	{
		open_matrix_SPI();
		print_string(menu_string);
		close_matrix_SPI();
		
		if (poll_switch() == SW_MODE)
		{
			beep(buzzer_state);
			while(poll_switch() == SW_MODE);
			out_confirm_x = 1;
		}
		if (poll_switch() == SW_ADJUST)
		{
			beep(buzzer_state);
			while(poll_switch() == SW_ADJUST);
			current_state = ladder_string_display(current_state, up);
		}
		else if (poll_switch() == SW_RGB)
		{
			beep(buzzer_state);
			while(poll_switch() == SW_RGB);
			current_state = ladder_string_display(current_state, down);
		}
	}
	return current_state;
}

/* Color menu function */
uint8_t menu_mode_color (void)
{
	int colorcnt = 0;
	uint8_t current_state = R_mode;
	strcpy(menu_string,REDST);
	uint8_t out_confirm_x = 0;
	while(out_confirm_x == 0)
	{
		open_matrix_SPI();
		print_string(menu_string);
		close_matrix_SPI();
		if (current_state == PWM_mode)
		{
			Init_RGB_LED();
			showRGB(colorcnt);
			if (colorcnt == 767) colorcnt = 0;
			colorcnt++;			
		}
		else if (current_state == R_mode)
		{
			stop_timers();
			PORTD |=  (1 << RED);
			PORTD &= ~(1 << GREEN);
			PORTB &= ~(1 << BLUE);
		}
		else if (current_state == G_mode)
		{
			stop_timers();
			PORTD &= ~(1 << RED);
			PORTD |= (1 << GREEN);
			PORTB &= ~(1 << BLUE);
		}
		else if (current_state == B_mode)
		{
			stop_timers();
			PORTD &= ~(1 << RED);
			PORTD &= ~(1 << GREEN);
			PORTB |= (1 << BLUE);
		}
		else if (current_state == off_mode)
		{
			stop_timers();
			PORTD &= ~(1 << RED);
			PORTD &= ~(1 << GREEN);
			PORTB &= ~(1 << BLUE);
		}		
		
		if (poll_switch() == SW_MODE)
		{
			beep(buzzer_state);
			while(poll_switch() == SW_MODE);
			out_confirm_x = 1;
		}
		if (poll_switch() == SW_ADJUST)
		{
			beep(buzzer_state);
			while(poll_switch() == SW_ADJUST);
			current_state = ladder_string_color(current_state, up);
		}
		else if (poll_switch() == SW_RGB)
		{
			beep(buzzer_state);
			while(poll_switch() == SW_RGB);
			current_state = ladder_string_color(current_state, down);
		}
	}
	return current_state;
}

/* Time and date adjust menu mode function */
uint8_t adjust_mode (uint8_t parameter_in)
{
	uint8_t ret_value = 0;
	out_confirm = 0;
	switch(parameter_in)
	{
		case SEC_state:
			while(out_confirm == 0) // Wait until OK
			{
				open_matrix_SPI();
				print_string("<SEC:");
				int_to_ascii(ret_value);
				Print_Character(buf[0]);
				Print_Character(buf[1]);
				Print_Character('>');
				close_matrix_SPI();
				if (poll_switch() == SW_MODE) // Exit from state
				{
					beep(buzzer_state);
					while(poll_switch() == SW_MODE);
					out_confirm = 1;
				}
			    if (poll_switch() == SW_ADJUST) 
				{
					beep(buzzer_state);
					while(poll_switch() == SW_ADJUST);
					if (ret_value > 59)  ret_value = 0;
					else ret_value++;
				}
				else if (poll_switch() == SW_RGB) // OK and save appropriate values.
				{
					beep(buzzer_state);
					while(poll_switch() == SW_RGB);
					if (ret_value <= 0)  ret_value = 59;
					else ret_value--;
				}
			}
			break;
		
			case MIN_state:
			while(out_confirm == 0) // Wait until OK
			{
				open_matrix_SPI();
				print_string("<MIN:");
				int_to_ascii(ret_value);
				Print_Character(buf[0]);
				Print_Character(buf[1]);
				Print_Character('>');
				close_matrix_SPI();
				if (poll_switch() == SW_MODE) // Exit from state
				{			
					beep(buzzer_state);		
					while(poll_switch() == SW_MODE);
					out_confirm = 1;
				}
				else if (poll_switch() == SW_ADJUST) 
				{
					beep(buzzer_state);
					while(poll_switch() == SW_ADJUST);
					if (ret_value > 59)  ret_value = 0;
					else ret_value++;
				}
				else if (poll_switch() == SW_RGB) // OK and save appropriate values.
				{
					beep(buzzer_state);
					while(poll_switch() == SW_RGB);
					if (ret_value <= 0)  ret_value = 59;
					else ret_value--;
				}
			}	
			break;
		
		case HOUR_state:
		while(out_confirm == 0) // Wait until OK
		{
			open_matrix_SPI();
			print_string("<HR: ");
			int_to_ascii(ret_value);
			Print_Character(buf[0]);
			Print_Character(buf[1]);
			Print_Character('>');
			close_matrix_SPI();
				if (poll_switch() == SW_MODE) // Exit from state
				{
					beep(buzzer_state);
					while(poll_switch() == SW_MODE);
					out_confirm = 1;
				}
				else if (poll_switch() == SW_ADJUST) 
				{
					beep(buzzer_state);
					while(poll_switch() == SW_ADJUST);
				if (ret_value > 23)  ret_value = 0;
				else ret_value++;
			}
			else if (poll_switch() == SW_RGB) // OK and save appropriate values.
			{
				beep(buzzer_state);
				while(poll_switch() == SW_RGB);
				if (ret_value <= 0)  ret_value = 23;
				else ret_value--;
			}
		}
		break;
		
		case DAY_state:
		while(out_confirm == 0) // Wait until OK
		{
			buf[0] = '1';
			buf[1] = '0';
			open_matrix_SPI();
			print_string("<DAY:");
			int_to_ascii(ret_value);
			Print_Character(buf[0]);
			Print_Character(buf[1]);
			Print_Character('>');
			close_matrix_SPI();
				if (poll_switch() == SW_MODE) // Exit from state
				{
					beep(buzzer_state);
					while(poll_switch() == SW_MODE);
					out_confirm = 1;
				}
				else if (poll_switch() == SW_ADJUST) 
				{
					beep(buzzer_state);
					while(poll_switch() == SW_ADJUST);
				if (ret_value > 31)  ret_value = 1;
				else ret_value++;
			}
			else if (poll_switch() == SW_RGB) // OK and save appropriate values.
			{
				beep(buzzer_state);
				while(poll_switch() == SW_RGB);
				if (ret_value <= 1)  ret_value = 31;
				else ret_value--;
			}
		}
		break;
		
		case MONTH_state:
		while(out_confirm == 0) // Wait until OK
		{
			buf[0] = '1';
			buf[1] = '0';
			open_matrix_SPI();
			print_string("<MTH:");
			int_to_ascii(ret_value);
			Print_Character(buf[0]);
			Print_Character(buf[1]);
			Print_Character('>');
			close_matrix_SPI();
				if (poll_switch() == SW_MODE) // Exit from state
				{
					beep(buzzer_state);
					while(poll_switch() == SW_MODE);
					out_confirm = 1;
				}
				else if (poll_switch() == SW_ADJUST) 
				{
					beep(buzzer_state);
					while(poll_switch() == SW_ADJUST);
				if (ret_value > 12)  ret_value = 1;
				else ret_value++;
			}
			else if (poll_switch() == SW_RGB) // OK and save appropriate values.
			{
				beep(buzzer_state);
				while(poll_switch() == SW_RGB);
				if (ret_value <= 1)  ret_value = 12;
				else ret_value--;
			}
		}
		break;	
			
		case YEAR_state:
		while(out_confirm == 0) // Wait until OK
		{
			open_matrix_SPI();
			print_string("<Y:20");
			int_to_ascii(ret_value);
			Print_Character(buf[0]);
			Print_Character(buf[1]);
			Print_Character('>');
			close_matrix_SPI();
				if (poll_switch() == SW_MODE) // Exit from state
				{
					beep(buzzer_state);
					while(poll_switch() == SW_MODE);
					out_confirm = 1;
				}
				else if (poll_switch() == SW_ADJUST)
				{
					beep(buzzer_state);
					while(poll_switch() == SW_ADJUST);
				if (ret_value > 99)  ret_value = 0;
				else ret_value++;
			}
			else if (poll_switch() == SW_RGB) // OK and save appropriate values.
			{
				beep(buzzer_state);
				while(poll_switch() == SW_RGB);
				if (ret_value <= 0)  ret_value = 99;
				else ret_value--;
			}
		}
		break;						
	}
	return ret_value;
}

/* Enable Alarm LED #1 */
void enable_led_alarm(void)
{
	stop_timers();
	PORTD |=  (1 << RED);
	PORTD &= ~(1 << GREEN);
	PORTB &= ~(1 << BLUE);
}

/* Enable Alarm LED #2 */
void disable_led_alarm(void)
{
	stop_timers();
	PORTD &= ~(1 << RED);
	PORTD |=  (1 << GREEN);
	PORTB &= ~(1 << BLUE);
}

/* Enable Alarm */
void alarm_begin (void)
{
	for (uint8_t ixz = 0; ixz < 60; ixz++)
	{
		if (poll_switch() == SW_MODE) 
		{
			while(poll_switch() == SW_MODE);
		break;
		}
		open_matrix_SPI();
		print_string("WAKE UP ");
		close_matrix_SPI();
		enable_led_alarm();
		for (uint16_t xin = 0; xin < 1000; xin++)
		{
			PORTD |= (1 << 0);
			_delay_us(250);
			PORTD &=~(1 << 0);
			_delay_us(250);
			
		}
		_delay_ms(50);
		disable_led_alarm();
		open_matrix_SPI();
		print_string("  NOW!  ");
		close_matrix_SPI();
		_delay_ms(300);
	}
	Init_RGB_LED();
}

/* Buzzer single "beep" generation */
void beep (uint8_t buzzer_in)
{
	for (uint16_t sn = 0; sn < 50; sn++)
	{
		if (buzzer_in == buzzer_on)
		{
			PORTD |= (1 << 0);
			_delay_us(250);
			PORTD &=~(1 << 0);
			_delay_us(250);
		}
		else ;
	}
}

/* Buzzer menu switching between states */
uint8_t ladder_string_buzzer(uint8_t current_statez, uint8_t operator_in)
{
	uint8_t retvalue = 0;
	switch(current_statez)
	{
		case buzzer_on:
		if (operator_in == up)
		{
			retvalue = buzzer_off;
			strcpy(menu_string, OFFST);
		}
		if (operator_in == down)
		{
			retvalue = buzzer_off;
			strcpy(menu_string, OFFST);
		}
		break;
		
		case buzzer_off:
		if (operator_in == up)
		{
			retvalue = buzzer_on;
			strcpy(menu_string, ONST);
		}
		if (operator_in == down)
		{
			retvalue = buzzer_on;
			strcpy(menu_string, ONST);
		}
		break;
	}
	return retvalue;
}

/* Buzzer menu mode function */
uint8_t menu_mode_buzzer (void)
{
	uint8_t current_state = buzzer_on;
	//	uint8_t ret_val = 0;
	strcpy(menu_string,ONST);
	uint8_t out_confirm_x = 0;
	while(out_confirm_x == 0)
	{
		open_matrix_SPI();
		print_string(menu_string);
		close_matrix_SPI();
		
		if (poll_switch() == SW_MODE)
		{
			beep(buzzer_state);
			while(poll_switch() == SW_MODE);
			out_confirm_x = 1;
		}
		if (poll_switch() == SW_ADJUST)
		{
			beep(buzzer_state);
			while(poll_switch() == SW_ADJUST);
			current_state = ladder_string_buzzer(current_state, up);
		}
		else if (poll_switch() == SW_RGB)
		{
			beep(buzzer_state);
			while(poll_switch() == SW_RGB);
			current_state = ladder_string_buzzer(current_state, down);
		}
	}
	buzzer_state = current_state;
	return current_state;
}

/* Main function */
int main (void)
{	
	Init_Device();						// Device initialization
	unsigned char sec, temp;			// used variables
	uint16_t color_cnt = 0;				// used variable
	unsigned long display_cnt = 0;		// used variable
	uint8_t RGB_out = PWM_mode;			// Default RGB mode.
	uint8_t DISPLAY_out = time_mode;	// Default time displaying mode.
	struct rtc_time ds1302;				// DS1302 semi-"class" Definition
	struct rtc_time *rtc;
	rtc = &ds1302;
	ds1302_init();						// DS1302 initialization
	ds1302_update(rtc);					// update all fields in the struct
	ds1302_update_time(rtc, HOUR);		// Get HOUR value
	greetings(rtc->hour);				// Greetings according to the daytime.
	while (1)							// Infinite loop
	{
		if (poll_switch() == SW_MODE) // Check PB state
		{
			beep(buzzer_state);
			while(poll_switch() == SW_MODE);
			switch(menu_mode())
			{
				case MENU_ADJUST:
					ds1302_set_time(rtc, SEC, adjust_mode(SEC_state));
					ds1302_set_time(rtc, MIN, adjust_mode(MIN_state));
					ds1302_set_time(rtc, HOUR, adjust_mode(HOUR_state));
					_delay_ms(50);
					ds1302_set_time(rtc, DATE, adjust_mode(DAY_state));
					_delay_ms(50);
					ds1302_set_time(rtc, MONTH, adjust_mode(MONTH_state));
					_delay_ms(50);					
					break;
					
				case MENU_COLORS:
					RGB_out = menu_mode_color();
					break;
					
				case MENU_DISPLAY:
					DISPLAY_out = menu_mode_display();
					break;
					
				case MENU_ALARM:
					sec_alarm = adjust_mode(SEC_state);
					min_alarm = adjust_mode(MIN_state);
					hour_alarm = adjust_mode(HOUR_state);
					break;
					
				case MENU_BUZZER:
					buzzer_state = menu_mode_buzzer();
					break;
					
				default: break;
			}
		}
		
/* RGB LED Controller */
		switch(RGB_out)
		{
			case R_mode: 
			stop_timers();
				PORTD |=  (1 << RED);
				PORTD &= ~(1 << GREEN);
				PORTB &= ~(1 << BLUE);
				break;
			case G_mode:
			stop_timers();
				PORTD &=  ~(1 << RED);
				PORTD |= (1 << GREEN);
				PORTB &= ~(1 << BLUE);
				break;
			case B_mode:
			stop_timers();
				PORTD &=  ~(1 << RED);
				PORTD &= ~(1 << GREEN);
				PORTB |= (1 << BLUE);
				break;
			case PWM_mode:
				showRGB(color_cnt);
				if (color_cnt == 767) color_cnt = 0;
				color_cnt++;
				break;
			case off_mode:
				stop_timers();
				PORTD &= ~(1 << RED);
				PORTD &= ~(1 << GREEN);
				PORTB &= ~(1 << BLUE);	
				break;
			default: break;						
		}

/* Display Controller */

		switch(DISPLAY_out)
		{
			case time_mode:
			ds1302_update_time(rtc, SEC);
			ds1302_update_time(rtc, MIN);
			ds1302_update_time(rtc, HOUR);
				open_matrix_SPI();
				print_time_string(rtc->second,rtc->minute,rtc->hour);
				close_matrix_SPI();	
				break;
				
			case date_mode:
				ds1302_update_time(rtc, DATE);
				ds1302_update_time(rtc, MONTH);
				open_matrix_SPI();
				print_date_string(rtc->date,rtc->month);
				close_matrix_SPI();
				break;
				
			case both_mode:

				open_matrix_SPI();
				if (display_cnt < 400) 
				{
				ds1302_update_time(rtc, SEC);
				ds1302_update_time(rtc, MIN);
				ds1302_update_time(rtc, HOUR);
				print_time_string(rtc->second,rtc->minute,rtc->hour);
				display_cnt++;
				}
				if (display_cnt >= 400)
				{
					ds1302_update_time(rtc, DATE);
					ds1302_update_time(rtc, MONTH);
					display_cnt++;
					print_date_string(rtc->date,rtc->month);
					if (display_cnt == 800) display_cnt = 0;
					
				}
				close_matrix_SPI();
				break;
				
			default: break;						
		}
		
		/* Alarm compare code block */
		if ( (hour_alarm == rtc->hour) && (min_alarm == rtc->minute) && (sec_alarm == rtc->second) ) alarm_begin();
	}
	return 0;
}