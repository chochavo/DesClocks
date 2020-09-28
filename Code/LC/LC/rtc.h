#ifndef _RTC_H_
#define _RTC_H_

/* DS1302 I/O Definitions */
#define ds1302_PORT PORTB
#define ds1302_DDR DDRB
#define ds1302_PIN PINB
#define rst 7
#define clk 6
#define io 0

#define rst_1() ds1302_PORT |= (1<<rst);
#define rst_0() ds1302_PORT &= ~ (1<<rst);
#define clk_1() ds1302_PORT |= (1<<clk);
#define clk_0() ds1302_PORT &= ~ (1<<clk);
#define io_1() 	ds1302_PORT |= (1<<io);
#define io_0() 	ds1302_PORT &= ~ (1<<io);

/* DS1302 Format constants */
#define AM		10
#define PM		11
#define H24		12

/* DS1302 Data directions */
#define READ	15
#define WRITE	16

/* DS1302 date constants */
#define mon 	1
#define tue 	2
#define wed 	3
#define thu 	4
#define fri 	5
#define sat 	6
#define sun 	7

/* DS1302 R\W Register values */
#define sec_w 	0x80
#define sec_r 	0x81
#define min_w 	0x82
#define min_r 	0x83
#define hour_w 	0x84
#define hour_r 	0x85
#define date_w 	0x86
#define date_r 	0x87
#define month_w 0x88
#define month_r 0x89
#define day_w 	0x8a
#define day_r 	0x8b
#define year_w 	0x8c
#define year_r 	0x8d

/* DS1302 appeal constants */
#define SEC 20
#define MIN 21
#define HOUR 22
#define DAY 22
#define DATE 23
#define MONTH 24
#define YEAR 25

#define w_protect 0x8e

/* Main RTC module structure */
struct rtc_time{
	char second;
	char minute;
	char hour;
	char day;
	char date;
	char month;
	char year;
	char hour_format;
};

/* Function prototypes */
void ds1302_update_time(struct rtc_time *, unsigned char);
void ds1302_set_time(struct rtc_time * time, unsigned char field, unsigned char w_byte);
void ds1302_comms(struct rtc_time *, unsigned char, unsigned char, unsigned char);
void ds1302_update(struct rtc_time *);
void ds1302_init (void); //ds1302 init
void ds1302_reset(void);	 //ds1302_reset
unsigned char ds1302_read_byte(unsigned char);
void ds1302_write_byte(unsigned char, unsigned char);
void write(unsigned char);
unsigned char read(void);
void ds1302_write_byte(unsigned char,unsigned char);

#endif