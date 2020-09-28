#ifndef DISPLAY_H_
#define DISPLAY_H_

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
/* LED Matrices pin declaration constants */
#define CLK 0x02 // PC2
#define DIN 0x00 // PC0
#define RS  0x01 // PC1
#define nCE 0x03 // PC3

/* ==== Defined Constants: ==== */
#define number_of_displays 8 // Total number of AVAGO displays
#define total_LEDS_number 320 // Total number of LEDs on display

/* Function Prototypes */
void clk_period(void);
void parallel_to_serial_matrix(uint8_t par_register);
void transmit_to_matrix(uint8_t ax, uint8_t bx, uint8_t cx, uint8_t dx, uint8_t ex);
void Print_Character(char sent_character);
void print_string(char string_array[]);
void Init_LED_MATRIX(void);
void Init_Display(void);
void open_matrix_SPI(void);
void close_matrix_SPI(void);

#endif /* DISPLAY_H_ */