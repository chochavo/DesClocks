#ifndef LC_H_
#define LC_H_

/* Menu constants */
#define MENU_ADJUST	 100
#define MENU_COLORS	 101
#define MENU_ALARM	 102
#define MENU_DISPLAY 103
#define MENU_BUZZER	 104
#define MENU_EXIT	 105

/* Switching between states in menus constants */
#define up 5
#define down 6

/* General purpose constants */
#define GPIO 5
#define write_protect 6

/* RGB pin declaration constants */
#define RED 5 //PD5 - OC0B
#define GREEN 6 //PD6 - OC0A
#define BLUE 1 // PB1 OC1A

/* Switches pin declaration constants */
#define SW_MODE 1 // PD1
#define SW_ADJUST	2 // PD2
#define SW_RGB 3 // PD3

/* RTC values adjust state machine values */
#define SEC_state 30
#define MIN_state 31
#define HOUR_state 32
#define DAY_state 33
#define MONTH_state 34
#define YEAR_state 35

/* RGB LED output modes constants */
#define PWM_mode 90
#define R_mode 91
#define G_mode 92
#define B_mode 93
#define off_mode 94

/* Display modes constants */
#define time_mode 95
#define date_mode 96
#define both_mode 97

/* Buzzer modes constants */
#define buzzer_on 98
#define buzzer_off 99

/* Main menu static strings to be shown */
static char COLORS[]	  = "<COLORS>";
static char ADJUST[]	  = "<ADJUST>";
static char ALARM[]		  = "<ALARM >";
static char BUZZER[]	  = "<BUZZER>";
static char DISPLAY[]	  = "<SHOW..>";
static char EXIT_MENU[]   = "< EXIT >";

/* Display menu static strings */
static char TIMEST[] = "< TIME >";
static char DATEST[] = "< DATE >";
static char BOTHST[] = "< BOTH >";

/* RGB LED static strings */
static char REDST[]		= "< RED  >";
static char GREENST[]	= "<GREEN >";
static char BLUEST[]	= "< BLUE >";
static char PWMST[]		= "< PWM  >";
static char OFFST[]		= "< OFF  >";
static char ONST[]		= "<  ON  >";

char menu_string[] = "<ADJUST>";

/* Function prototypes */
void Init_Device(void);
void alphabet_check(void);
void int_to_ascii(int xxx);
void int_to_ascii_long(int yyy);
void print_time_string(int second_in, int minute_in, int hour_in);
void print_date_string(int day_in, int month_in);
void Init_switches(void);
void Init_RGB_LED(void);
void PWM_OUT(int color_type, int duty_cycle);
void showRGB(int color);
uint8_t poll_switch (void);
uint8_t ladder_string(uint8_t current_state, uint8_t operator_in);
uint8_t ladder_string_color(uint8_t current_state, uint8_t operator_in);
uint8_t ladder_string_display(uint8_t current_state, uint8_t operator_in);
uint8_t menu_mode (void);
uint8_t menu_mode_display(void);
uint8_t menu_mode_color (void);
uint8_t adjust_mode (uint8_t parameter_in);
void beep (uint8_t buzzer_in);

#endif /* LC_H_ */