/* ==== Pin Description: ====		
1 - DOUT	- out
2 - OSC		- out
3 - VLED	- 5V
4 - DIN		- PD0 2
5 - RS		- PD1 3 
6 - CLK		- PD2 4 
7 - nCE		- PD3 5
8 - BLANK	- 0V
9 - GND		- 0V
10 - SEL	- '0'(EXT.), '1'(Int.).
11 - VLOGIC - 5V
12 - nRST	- Power-On of Circuit.
==== Power-on sequence: ====
1. Reset.
2. Dot Register <= zeros.
3. Control reg <= NO SLEEP + brightness.

 - write cycle: -
1. RS High\Low
2. nCE low
3. ---SPI TRANSFER (rising edge clk)
4. clk low.
5. nCE HIGH
*/

/* ==== Defined Constants: ==== */
#define number_of_displays 8
#define total_LEDS_number 320

#define CLK 0x02

#define DIN 0x00
#define RS  0x01
#define nCE 0x03

#include <avr/io.h>
#include <util/delay.h>

#ifndef __5X7_MATRIX_H
#define __5X7_MATRIX_H


void clk_period(void)
{
	PORTC |= (1 << CLK);
	_delay_us(10);
	PORTC &= ~(1 << CLK);
	_delay_us(10);
}

void parallel_to_serial_matrix(uint8_t par_register)
{
	for (uint8_t bits_loop = 0; bits_loop < 8; bits_loop++)
	{
		if (par_register & 0x80) { PORTC |= ( 1 << DIN); } // If MSB of current par_register '1' Then put '1' on DIN.
		else { PORTC &= ~(1 << DIN); }
		clk_period();
		par_register <<= 1; // Our transfer is MSB-first based, so we have to shift register left for every cycle.
	}
}

void transmit_to_matrix(uint8_t ax, uint8_t bx, uint8_t cx, uint8_t dx, uint8_t ex) // Here, we recieve all values that we'll want to transmit - (index defines number of row).
	{
		uint8_t pre_matrix[5];
		pre_matrix[0] = ax;
		pre_matrix[1] = bx;
		pre_matrix[2] = cx;
		pre_matrix[3] = dx;
		pre_matrix[4] = ex;
		
		for (uint8_t array_loop = 0; array_loop < 5; array_loop++) //Total 5 rows. 
		{
			
			parallel_to_serial_matrix(pre_matrix[array_loop]);
		}
	}


void Print_Character(char sent_character) // Here we recieve desired character
	{
		switch(sent_character) //Lower-case & Upper-case letters.
		{
			/*
			case 'a': transmit_to_matrix() 
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
			case 'g':
			case 'h':
			case 'i':
			case 'j':
			case 'k':
			case 'l':
			case 'm':
			case 'n':			
			case 'o':
			case 'p':
			case 'q':
			case 'r':
			case 's':
			case 't':
			case 'u':
			case 'v':
			case 'w':
			case 'x':
			case 'y':
			case 'z':
			*/
			case ' ': transmit_to_matrix(0x00,0x00,0x00,0x00,0x00); break;
			case 'A': transmit_to_matrix(0x7E,0x09,0x09,0x09,0x7E); break;
			case 'B': transmit_to_matrix(0x7F,0x49,0x49,0x49,0x76); break;
			case 'C': transmit_to_matrix(0x3E,0x41,0x41,0x41,0x22); break;
			case 'D': transmit_to_matrix(0x7F,0x41,0x41,0x41,0x7E); break;
			case 'E': transmit_to_matrix(0x7F,0x49,0x49,0x49,0x41); break;
			case 'F': transmit_to_matrix(0x7F,0x09,0x09,0x09,0x01); break;
			case 'G': transmit_to_matrix(0x7E,0x41,0x49,0x49,0x39); break;
			case 'H': transmit_to_matrix(0x7F,0x08,0x08,0x08,0x7F); break;
			case 'I': transmit_to_matrix(0x00,0x41,0x7F,0x41,0x00); break;
			case 'J': transmit_to_matrix(0x00,0x20,0x40,0x40,0x7F); break;
			case 'K': transmit_to_matrix(0x7F,0x08,0x14,0x22,0x41); break;
			case 'L': transmit_to_matrix(0x7F,0x40,0x40,0x40,0x40); break;
			case 'M': transmit_to_matrix(0x7F,0x02,0x04,0x02,0x7F); break;
			case 'N': transmit_to_matrix(0x7F,0x04,0x08,0x10,0x7F); break;
			case 'O': transmit_to_matrix(0x3E,0x41,0x41,0x41,0x3E); break;
			case 'P': transmit_to_matrix(0x7F,0x09,0x09,0x09,0x07); break;
			case 'Q': transmit_to_matrix(0x3E,0x41,0x51,0x61,0x7E); break;
			case 'R': transmit_to_matrix(0x7F,0x09,0x19,0x29,0x46); break;
			case 'S': transmit_to_matrix(0x4F,0x49,0x49,0x49,0x79); break;
			case 'T': transmit_to_matrix(0x01,0x01,0x7F,0x01,0x01); break; 
			case 'U': transmit_to_matrix(0x3F,0x40,0x40,0x40,0x3F); break;
			case 'V': transmit_to_matrix(0x1F,0x20,0x40,0x20,0x1F); break;
			case 'W': transmit_to_matrix(0x3F,0x40,0x30,0x40,0x3F); break;
			case 'X': transmit_to_matrix(0x63,0x14,0x08,0x14,0x63); break;
			case 'Y': transmit_to_matrix(0x07,0x08,0x78,0x08,0x07); break;
			case 'Z': transmit_to_matrix(0x61,0x51,0x49,0x45,0x43);	break;
			
			case '0': transmit_to_matrix(0x3E,0x41,0x41,0x41,0x3E); break;
			case '1': transmit_to_matrix(0x00,0x00,0x04,0x02,0x7F); break;
			case '2': transmit_to_matrix(0x46,0x61,0x51,0x49,0x46); break;
			case '3': transmit_to_matrix(0x22,0x41,0x49,0x49,0x36); break;
			case '4': transmit_to_matrix(0x18,0x14,0x12,0x7F,0x10); break;
			case '5': transmit_to_matrix(0x27,0x45,0x45,0x45,0x39); break;
			case '6': transmit_to_matrix(0x3E,0x49,0x49,0x49,0x32); break;
			case '7': transmit_to_matrix(0x01,0x01,0x79,0x05,0x03); break;
			case '8': transmit_to_matrix(0x36,0x49,0x49,0x49,0x36); break;
			case '9': transmit_to_matrix(0x06,0x49,0x49,0x49,0x3E); break;
			
			case '=': transmit_to_matrix(0x24,0x24,0x24,0x24,0x24); break;
			case '.': transmit_to_matrix(0x00,0x60,0x60,0x00,0x00); break;
			
			default: transmit_to_matrix(0x63,0x14,0x08,0x14,0x63); break;

		}
	} 

void print_string(char string_array[])
{
	PORTC &= ~(1 << RS);
	PORTC &= ~(1 << nCE); // Start serial transmitting.
	for (uint8_t character_loop = 0; character_loop < number_of_displays; character_loop++)
	{
		Print_Character(string_array[character_loop]);
	}
	PORTC &= ~(1 << CLK); // Bring clk to LOW, so the data can be latched into the LEDS.
	PORTC |=  (1 << nCE); // Bring nCE to HIGH means - stop serial communication (End of package).
	PORTC &= ~(1 << DIN);
}

#endif