/*   DS1305DRV.C
     Driver for Real Time Clock DS1305
     Vers 1.0

- notes:
  1) Uses hardware SPI

*/

// ===========================================
// PROTOTYPES
// ===========================================

void rtc_init();      // init RTC with 1 second and 1 day alarms
void rtc_init_lite(); // init RTC without alarms

void rtc_set_datetime(                     // writes date/time inside RTC
                      unsigned char dow,    // dow = week day (1-7)
                      unsigned char day,    // day = day (1 -31)
                      unsigned char mth,    // mth = month (1 - 12)
                      unsigned char year,   // year = year 0 - 99 (to be added to 2000)
                      unsigned char hour,   // hour = hour (0 - 23) (24hr format)
                      unsigned char min,    // min = minutes 0 - 59
                      unsigned char sec);   // sec = seconds 0 - 59

void rtc_get_date(                         // reads date
                  unsigned char *dow,       // dow = giorno della settimana 1-7
                  unsigned char *day,       // day = giorno 1 -31
                  unsigned char *mth,       // mth = mese 1 - 12
                  unsigned char *year);     // year = anno 0 - 99 (da sommare a 2000)


void rtc_get_time(                         // reads time
                  unsigned char *hr,        // hour = ora 0 - 23 (su 24 ore)
                  unsigned char *min,       // min = minuti 0 - 59
                  unsigned char *sec);      // sec = secondi 0 - 59

void rtc_write_nvr(unsigned char address, unsigned char data); // writes a byte on the internal NVRAM
                                                             // address =  0x00 - 0x5F

unsigned char rtc_read_nvr(unsigned char address) ; // reads a byte on the internal NVRAM
                                                  // address = 0x00 - 0x5F

// FUNZIONI INTERNE
void set_CE_rtc();
void reset_CE_rtc();
void write_rtc_byte(unsigned char add, unsigned char data);
unsigned char read_rtc_byte(unsigned char add);
unsigned char get_bcd(unsigned char data);
unsigned char rm_bcd(unsigned char data);

// ===================================================
// PROTOTYPES END
// ===================================================

// definisce il pin di chip enable al RTC
#define RTC_ENABLE    PORTH.F6     //su porta H
#define T_RTC         2            //ritardi temporizzazione RTC DALLAS in us(era =2us)

#define DAY_MTH_MASK     0x3F
#define DOW_MASK         0x0F

#define ALARM_MASK_ON    0x80
#define ALARM_MASK_OFF   0x00
/* ==================================================== */

void rtc_init() {
     Spi_Init_Advanced(MASTER_OSC_DIV16, DATA_SAMPLE_MIDDLE, CLK_IDLE_HIGH, LOW_2_HIGH);
     set_CE_rtc();
     reset_CE_rtc();
     delay_us(10);
     write_rtc_byte(0x8F,0x87);  // non attiva il clock, disattiva la protezione in scrittura
                                 // e abilita allarmi. I SEGNALI DI INTERRUPT RIMANGONO
                                 // abilitati fino a quando non viene gestito l'interrupt
                                 // e l'orologio lo rileva con un accesso ai registri!!
                                 // QUALSIASI LETTURA AZZERA GLI INTERRUPT ATTIVI
     write_rtc_byte(0x87,ALARM_MASK_ON);
     write_rtc_byte(0x88,ALARM_MASK_ON);
     write_rtc_byte(0x89,ALARM_MASK_ON);
     write_rtc_byte(0x8A,ALARM_MASK_ON);  // imposta allarme su INT0 = ogni secondo
     write_rtc_byte(0x8B,ALARM_MASK_OFF);
     write_rtc_byte(0x8C,ALARM_MASK_OFF);
     write_rtc_byte(0x8D,ALARM_MASK_OFF);
     write_rtc_byte(0x8E,ALARM_MASK_ON);  // imposta allarme su INT1 = ogni giorno
     write_rtc_byte(0x8F,0x07);  // attiva il clock e lascia invariati gli altri settaggi
}
/* ==================================================== */
void rtc_init_lite() {
     Spi_Init_Advanced(MASTER_OSC_DIV16, DATA_SAMPLE_MIDDLE, CLK_IDLE_HIGH, LOW_2_HIGH);
     set_CE_rtc();
     reset_CE_rtc();
     delay_us(10);
     write_rtc_byte(0x8F,0x0); // attiva il clk, disattiva la protezione in scrittura
                               // disattiva gli allarmi
}
/* ==================================================== */
void rtc_set_datetime(
                      unsigned char dow,
                      unsigned char day,
                      unsigned char mth,
                      unsigned char year,
                      unsigned char hour,
                      unsigned char min,
                      unsigned char sec){
                      
   write_rtc_byte(0x83, get_bcd(dow));
   write_rtc_byte(0x84, get_bcd(day));
   write_rtc_byte(0x85, get_bcd(mth));
   write_rtc_byte(0x86, get_bcd(year));
   write_rtc_byte(0x82, get_bcd(hour));
   write_rtc_byte(0x81, get_bcd(min));
   write_rtc_byte(0x80, get_bcd(sec));
}

/* ==================================================== */

void rtc_get_date(
                  unsigned char *dow,
                  unsigned char *day,
                  unsigned char *mth,
                  unsigned char *year) {
   
   *dow = read_rtc_byte(0x03) & DOW_MASK;
   *day = rm_bcd(read_rtc_byte(0x04) & DAY_MTH_MASK);
   *mth = rm_bcd(read_rtc_byte(0x05) & DAY_MTH_MASK);
   *year = rm_bcd(read_rtc_byte(0x06));
}
/* ==================================================== */

void rtc_get_time(
                  unsigned char *hr,
                  unsigned char *min,
                  unsigned char *sec) {

   *hr =  rm_bcd(read_rtc_byte(0x02));
   *min = rm_bcd(read_rtc_byte(0x01));
   *sec = rm_bcd(read_rtc_byte(0x00));

}

/* ==================================================== */

void rtc_write_nvr(unsigned char address, unsigned char data) {

write_rtc_byte(address + 0xa0, data);
}

/* ==================================================== */

unsigned char rtc_read_nvr(unsigned char address) {

return(read_rtc_byte(address + 0x20));
}

/* ==================================================== */
/* ==================================================== */
/* ==================================================== */
/* ==================================================== */
void set_CE_rtc() {

     SSPCON1.SSPEN=0;
     Spi_Init_Advanced(MASTER_OSC_DIV16, DATA_SAMPLE_MIDDLE, CLK_IDLE_HIGH, LOW_2_HIGH);

     RTC_ENABLE = 1;
}

/* ==================================================== */
void reset_CE_rtc() {

     RTC_ENABLE = 0;
}
/* ==================================================== */
void write_rtc_byte(unsigned char add, unsigned char data) {

     set_CE_rtc();
     delay_us(T_RTC);
     spi_write(add);
     spi_write(data);
     delay_us(T_RTC);
     reset_CE_rtc();
}
/* ==================================================== */
unsigned char read_rtc_byte(unsigned char add) {
     unsigned char rdata;

     rtc_init();
     set_CE_rtc();
     delay_us(T_RTC);
         
     spi_write(add);
     rdata = spi_read(0);

     delay_us(T_RTC);
     reset_CE_rtc();
         
     return rdata;
}

/* ==================================================== */

unsigned char get_bcd(unsigned char data)
{
   unsigned char nibh;
   unsigned char nibl;

   nibh = data / 10;
   nibl = data - (nibh * 10);

   return((nibh << 4) | nibl);
}

/* ==================================================== */

unsigned char rm_bcd(unsigned char data)
{
   unsigned char res;

   res = (data >> 4) * 10;
   res += (data & 0x0F);

   return res;
}
