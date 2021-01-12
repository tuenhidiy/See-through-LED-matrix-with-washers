/*
See-through led matrix with M10 washers
By TUENHIDIY
Date: 2021-Jan-12
*/
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include "timezone.h"
#include "font3x5.h"
#include "font5x7.h"
#include "font8x8.h"

#define RowA3_Pin       12  // GPIO12 - D6
#define RowA2_Pin       2   // GPIO2  - D4
#define RowA1_Pin       0   // GPIO0  - D3
#define RowA0_Pin       4   // GPIO4  - D2


#define BLANK_Pin       16  // GPIO16 - D0
#define DATA_Pin        13  // GPIO13 - D7   
#define CLOCK_Pin       14  // GPIO14 - D5 
#define LATCH_Pin       5   // GPIO5  - D1

//#define BUTTON          15  // GPIO15 - D8

#define FONT3x5         0
#define FONT5x7         1
#define FONT8x8         2

#define BAM_RESOLUTION  4

const char *WIFI_NETWORK_NAME = "FTPTelecom";     // Change to your wifi network name
const char *WIFI_PASSWORD     = "12345689";       // Change to your wifi password

const char *TIME_SERVER       = "asia.pool.ntp.org";
int myTimeZone = VST;       // change this to your time zone (see in timezone.h)

time_t now;
struct tm *timeinfo;

int year;
int month;
int day;
int hour;
int mins;
int sec;
int day_of_week;

unsigned long samplingtimeh1 = 0;

byte row;
int BAM_Bit, BAM_Counter=0;     // Bit Angle Modulation variables to keep track of things
uint8_t R;

byte matrixBuffer[BAM_RESOLUTION][10];

void __attribute__((optimize("O0"))) DIY_SPI(uint8_t DATA);

void ICACHE_RAM_ATTR timer1_ISR(void);   
void LED(uint8_t X, uint8_t Y, uint8_t RR);
void clearscreen();
void rowscan(byte row);
byte getPixelChar(uint8_t x, uint8_t y, wchar_t ch, uint8_t font);
byte getPixelHString(uint16_t x, uint16_t y, wchar_t *p,uint8_t font);
unsigned int lenString(wchar_t *p);
int checkConstrains(int value, int min, int max);
void printChar(uint8_t x, uint8_t y, uint8_t For_color, uint8_t Bk_color, char ch);
void hScroll(uint8_t y, byte For_color, byte Bk_color, wchar_t *mystring, uint8_t font, uint8_t delaytime, uint8_t times, uint8_t dir);
void SCROLLYYYYMMDDHHMMSS();

wchar_t YYYYMMDD[34];
wchar_t HHMMSS[32];

void setup () 
{    
  WiFi.begin(WIFI_NETWORK_NAME, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }

  configTime(3600*myTimeZone, 0, TIME_SERVER);

  while (now < EPOCH_1_1_2019)
  {
    now = time(nullptr);
    delay(500);
  }
  
  row = 0;
  noInterrupts();
  
  pinMode(DATA_Pin, OUTPUT);
  pinMode(CLOCK_Pin, OUTPUT);
  pinMode(LATCH_Pin, OUTPUT);
  //pinMode(BLANK_Pin, OUTPUT);
  pinMode(RowA0_Pin, OUTPUT);
  pinMode(RowA1_Pin, OUTPUT);
  pinMode(RowA2_Pin, OUTPUT);
  pinMode(RowA3_Pin, OUTPUT);
  //pinMode(BUTTON, INPUT);
  
  timer1_isr_init();
  timer1_attachInterrupt(timer1_ISR);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  timer1_write(500);
  interrupts();
  clearscreen();
  
  if (WiFi.status() == WL_CONNECTED)
    {        
      hScroll(0, 15, 0, L"     WEMOS D1 R2 IS CONNECTED TO NTP TIME SERVER...     ", FONT8x8, 120, 1, 1);
    }
  clearscreen();  
  for (byte x=0; x<8; x++)
    {
      for (byte y=0; y<10; y++)
      {
        LED(x, y, 15);
        delay(200);
      }
    }
   for (byte y=0; y<10; y++)
    {
      for (byte x=0; x<8; x++)
      {
        LED(x, y, 0);
        delay(200);
      }
    }   
  clearscreen();
}    

void loop()
{   
    time(&now);
    timeinfo    = localtime(&now);
    year        = timeinfo->tm_year + 1900;
    month       = timeinfo->tm_mon + 1;
    day         = timeinfo->tm_mday;
    hour        = timeinfo->tm_hour;
    mins        = timeinfo->tm_min;
    sec         = timeinfo->tm_sec;
    day_of_week = timeinfo->tm_wday;
    YYYYMMDD[0] = ' ' ;
    YYYYMMDD[1] = ' ' ;
    YYYYMMDD[2] = ' ' ;
    YYYYMMDD[3] = ' ' ;
    YYYYMMDD[4] = ' ' ;
    YYYYMMDD[5] = ' ' ;
    YYYYMMDD[6] = ' ' ;
    YYYYMMDD[7] = ' ' ;
    YYYYMMDD[8] = 'D' ;
    YYYYMMDD[9] = 'a' ;
    YYYYMMDD[10] = 't' ;
    YYYYMMDD[11] = 'e' ;
    YYYYMMDD[12] = ':' ;
    YYYYMMDD[13] = ' ' ;
    YYYYMMDD[14] = ((year/1000) % 10) + 48 ;
    YYYYMMDD[15] = ((year/ 100) % 10) + 48 ;
    YYYYMMDD[16] = ((year/10) %10) + 48 ;
    YYYYMMDD[17] = (year %10) + 48 ;
    YYYYMMDD[18] = '-' ;
    YYYYMMDD[19] = ((month/10) %10) + 48 ;
    YYYYMMDD[20] = (month %10) + 48 ;
    YYYYMMDD[21] = '-' ;
    YYYYMMDD[22] = ((day/10) %10) + 48 ;
    YYYYMMDD[23] = (day %10) + 48 ;
    YYYYMMDD[24] = ' ' ;
    YYYYMMDD[25] = ' ' ;
    YYYYMMDD[26] = ' ' ;
    YYYYMMDD[27] = ' ' ;
    YYYYMMDD[28] = ' ' ;
    YYYYMMDD[29] = ' ' ;
    YYYYMMDD[30] = ' ' ;
    YYYYMMDD[31] = ' ' ;
    YYYYMMDD[32] = ' ' ;
    YYYYMMDD[33] = '\0' ;
  
    HHMMSS[0] = ' ' ;
    HHMMSS[1] = ' ' ;
    HHMMSS[2] = ' ' ;
    HHMMSS[3] = ' ' ;
    HHMMSS[4] = ' ' ;
    HHMMSS[5] = ' ' ;
    HHMMSS[6] = ' ' ;
    HHMMSS[7] = ' ' ;
    HHMMSS[8] = 'H' ;
    HHMMSS[9] = 'o' ;
    HHMMSS[10] = 'u' ;
    HHMMSS[11] = 'r' ;
    HHMMSS[12] = '-' ;
    HHMMSS[13] = ' ' ;
    HHMMSS[14] = ((hour/10) %10) + 48 ;
    HHMMSS[15] = (hour%10) + 48 ;
    HHMMSS[16] = ':' ;
    HHMMSS[17] = ((mins/10) %10) + 48 ;
    HHMMSS[18] = (mins%10) + 48 ;
    HHMMSS[19] = ':' ;
    HHMMSS[20] = ((sec/10) %10) + 48 ;
    HHMMSS[21] = (sec%10) + 48 ;
    HHMMSS[22] = ' ' ;
    HHMMSS[23] = ' ' ;
    HHMMSS[24] = ' ' ;
    HHMMSS[25] = ' ' ;
    HHMMSS[26] = ' ' ;
    HHMMSS[27] = ' ' ;
    HHMMSS[28] = ' ' ;
    HHMMSS[29] = ' ' ;
    HHMMSS[30] = ' ' ;
    HHMMSS[31] = '\0' ;

    SCROLLYYYYMMDDHHMMSS();
}

void LED(uint8_t X, uint8_t Y, uint8_t RR)
{
  
  uint8_t whichbyte = (Y + X/8);
  uint8_t whichbit = 7-(X % 8);
  
  for (byte BAM = 0; BAM < BAM_RESOLUTION; BAM++) 
  {
    bitWrite(matrixBuffer[BAM][whichbyte], whichbit, bitRead(RR, BAM));
  }
}


void clearscreen()
{
  memset(matrixBuffer, 0, sizeof(matrixBuffer[0][0]) * BAM_RESOLUTION * 10);
}

void rowscan(byte row)
{
  if (row & 0x08)  digitalWrite(RowA3_Pin,HIGH);
    else            digitalWrite(RowA3_Pin,LOW);

  if (row & 0x04)  digitalWrite(RowA2_Pin,HIGH);   
    else            digitalWrite(RowA2_Pin,LOW);          

  if (row & 0x02)  digitalWrite(RowA1_Pin,HIGH);   
    else            digitalWrite(RowA1_Pin,LOW);          

  if (row & 0x01)  digitalWrite(RowA0_Pin,HIGH);   
    else            digitalWrite(RowA0_Pin,LOW);  
    
}  
void __attribute__((optimize("O0"))) DIY_SPI(uint8_t DATA)
{
    uint8_t i;
    uint8_t val;
    for (i = 0; i<8; i++)  
    {
      digitalWrite(DATA_Pin, !!(DATA & (1 << (7 - i))));
      digitalWrite(CLOCK_Pin, HIGH);
      digitalWrite(CLOCK_Pin, LOW);                
    }
}


void ICACHE_RAM_ATTR timer1_ISR(void)
{   
  digitalWrite(BLANK_Pin, HIGH);

  if(BAM_Counter==8)    // Bit weight 2^0 of BAM_Bit, lasting time = 8 ticks x interrupt interval time
  BAM_Bit++;
  else
  if(BAM_Counter==24)   // Bit weight 2^1 of BAM_Bit, lasting time = 24 ticks x interrupt interval time
  BAM_Bit++;
  else
  if(BAM_Counter==56)   // Bit weight 2^3 of BAM_Bit, lasting time = 56 ticks x interrupt interval time
  BAM_Bit++;
  BAM_Counter++;
  switch (BAM_Bit)
    {
    case 0:
        DIY_SPI(matrixBuffer[0][row]);
      break;
    case 1:      
        DIY_SPI(matrixBuffer[1][row]);
      break;
    case 2:     
        DIY_SPI(matrixBuffer[2][row]);
      break;
    case 3:
        DIY_SPI(matrixBuffer[3][row]);
     
    if(BAM_Counter==120)    //Bit weight 2^3 of BAM_Bit, lasting time = 120 ticks x interrupt interval time
    {
    BAM_Counter=0;
    BAM_Bit=0;
    }
    break;
  }
  
  rowscan(row);
  digitalWrite(LATCH_Pin, HIGH);
  delayMicroseconds(2);
  digitalWrite(LATCH_Pin, LOW);
  delayMicroseconds(2);
  digitalWrite(BLANK_Pin, LOW);
  row++;
  if (row == 10) row=0;
  
  pinMode(BLANK_Pin, OUTPUT);
  timer1_write(500);
}

void fillRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t ON)
{
    for (uint16_t x = x1; x <= x2; x++) {
        for (uint16_t y = y1; y <= y2; y++) {
            LED(x, y, ON);      
        }
    }
}

byte getPixelChar(uint8_t x, uint8_t y, wchar_t ch, uint8_t font)

{
  if (font==FONT3x5)
  {
    if (x > 2) return 0; // 2 = font Width -1
    return bitRead(pgm_read_byte(&font3x5[ch-32][4-y]), 2-x); // 2 = Font witdh -1  
  }
  
  else if (font==FONT5x7)
  {
    if (x > 4) return 0; // 4 = font Width -1
    return bitRead(pgm_read_byte(&font5x7[ch-32][6-y]), 4-x); // 4 = Font witdh -1  
  }
  
  else if (font==FONT8x8)
  {
    if (x > 7) return 0; // 7 = font Width -1
    return bitRead(pgm_read_byte(&font8x8[ch-32][7-y]), 7-x); // 7 = Font witdh -1  
  }
  
}
byte getPixelHString(uint16_t x, uint16_t y, wchar_t *p,uint8_t font)

{
  if (font==FONT3x5)
  {
    p=p+x/4;
    return getPixelChar(x%4, y, *p, FONT3x5);
  }
  
  else if (font==FONT5x7)
  {
    p=p+x/6;
    return getPixelChar(x%6, y, *p, FONT5x7);
  }
  
  else if (font==FONT8x8)
  {
    p=p+x/8;
    return getPixelChar(x%8, y, *p, FONT8x8);  
  }
}

unsigned int lenString(wchar_t *p)
{
  unsigned int retVal=0;
  while(*p!='\0')
  { 
   retVal++;
   p++;
  }
  return retVal;
}


void printChar(uint8_t x, uint8_t y, uint8_t For_color, uint8_t Bk_color, char ch)
{
  uint8_t xx,yy;
  xx=0;
  yy=0;
    
  for (yy=0; yy < 5; yy++)
    {
    for (xx=0; xx < 3; xx++)
      {
      if (bitRead(pgm_read_byte(&font3x5[ch-32][4-yy]),2-xx))
      
        {
            LED(x+xx, y+yy, For_color);
        }
      else
        {
            LED(x+xx, y+yy, Bk_color);      
        }
      }
    }
}

void hScroll(uint8_t y, byte For_color, byte Bk_color, wchar_t *mystring, uint8_t font, uint8_t delaytime, uint8_t times, uint8_t dir)
{
  // FONT 5x7
  int offset;
  byte color;
  if (font == FONT3x5)
  {
  while (times)
    {
    for ((dir) ? offset=0 : offset=((lenString(mystring)-4)*4-1) ; (dir) ? offset <((lenString(mystring)-4)*4-1) : offset >0; (dir) ? offset++ : offset--)
      {
      for (byte xx=0; xx<8; xx++)
        {
        for (byte yy=0; yy<5; yy++)
            {            
              if (getPixelHString(xx+offset, yy, mystring, FONT3x5))
              {
                color = For_color;                
              }
              else 
              {
                color = Bk_color;
              }
                LED(xx, (yy+y), color);
            }
        }
        delay(delaytime);  
      }
    times--;
    }
  }
  
  else if (font == FONT5x7)
  {
  while (times)
    {
    for ((dir) ? offset=0 : offset=((lenString(mystring)-5)*6-1) ; (dir) ? offset <((lenString(mystring)-5)*6-1) : offset >0; (dir) ? offset++ : offset--)
      {
      for (byte xx=0; xx<8; xx++)
        {
        for (byte yy=0; yy<7; yy++)
            {            
              if (getPixelHString(xx+offset, yy, mystring, FONT5x7))
              {
                color = For_color;                
              }
              else 
              {
                color = Bk_color;
              }
                LED(xx, (yy+y), color);
            }
        }
        delay(delaytime);  
      }
    times--;
    }
  }   

// FONT 8x8
  else if (font == FONT8x8)
    {
    while (times)
      {
      for ((dir) ? offset=0 : offset=((lenString(mystring)-6)*8-1); (dir) ? offset <((lenString(mystring)-6)*8-1): offset >0; (dir) ? offset++ : offset--)
        {
        for (byte xx=0; xx<8; xx++)
          {
            for (byte yy=0; yy<8; yy++)
              {
                if (getPixelHString(xx+offset, yy, mystring, FONT8x8)) 
                  {
                  color = For_color;
                  }
                else 
                {
                  color = Bk_color;
                }
                  LED(xx, (yy+y), color);
              }          
            }
      delay(delaytime);  
        }
      times--;
      }
    }
 }

void SCROLLYYYYMMDDHHMMSS()
{ 

  hScroll(0, 15, 0, YYYYMMDD, FONT8x8, 120, 1, 1);
  hScroll(1, 15, 0, HHMMSS,FONT5x7, 120, 1, 1);

}
