/*********************************************************************
This is a library for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

These displays use SPI to communicate, 4 or 5 pins are required to
interface

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.
BSD license, check license.txt for more information
All text above, and the splash screen below must be included in any redistribution
*********************************************************************/

#ifdef __AVR__
  #include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
 #include <pgmspace.h>
#else
 #define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif

#if !defined(__ARM_ARCH) && !defined(ENERGIA) && !defined(ESP8266) && !defined(ESP32) && !defined(__arc__)
 #include <util/delay.h>
#endif

#include <stdlib.h>

//#include <Wire.h>
#include <HardWire.h>
#include "glcdfont.h"

#include "Adafruit_SSD1306.h"


extern HardWire _wire;//(1, I2C_FAST_MODE);


#define ssd1306_swap(a, b) { int16_t t = a; a = b; b = t; }


Adafruit_SSD1306::Adafruit_SSD1306(int8_t SID, int8_t SCLK, int8_t DC, int8_t RST, int8_t CS)
{
  cs = CS;
  rst = RST;
  dc = DC;
  sclk = SCLK;
  sid = SID;
  hwSPI = false;
}

// constructor for hardware SPI - we indicate DataCommand, ChipSelect, Reset
Adafruit_SSD1306::Adafruit_SSD1306(int8_t DC, int8_t RST, int8_t CS)
{
  dc = DC;
  rst = RST;
  cs = CS;
  hwSPI = true;
}

// initializer for I2C - we only indicate the reset pin!
Adafruit_SSD1306::Adafruit_SSD1306(int8_t reset)
{
 
  sclk = dc = cs = sid = -1;
  rst = reset;
}


void Adafruit_SSD1306::begin(uint8_t vccstate, uint8_t i2caddr, bool reset) {
  _vccstate = vccstate;
  _i2caddr = i2caddr;

  // set pin directions
  if (sid != -1){
    pinMode(dc, OUTPUT);
    pinMode(cs, OUTPUT);
#ifdef HAVE_PORTREG
    csport      = portOutputRegister(digitalPinToPort(cs));
    cspinmask   = digitalPinToBitMask(cs);
    dcport      = portOutputRegister(digitalPinToPort(dc));
    dcpinmask   = digitalPinToBitMask(dc);
#endif
    if (!hwSPI){
      // set pins for software-SPI
      pinMode(sid, OUTPUT);
      pinMode(sclk, OUTPUT);
#ifdef HAVE_PORTREG
      clkport     = portOutputRegister(digitalPinToPort(sclk));
      clkpinmask  = digitalPinToBitMask(sclk);
      mosiport    = portOutputRegister(digitalPinToPort(sid));
      mosipinmask = digitalPinToBitMask(sid);
#endif
      }
    if (hwSPI){
    }
  }
  else
  {
    // I2C Init
    _wire.begin();
#ifdef __SAM3X8E__
    // Force 400 KHz I2C, rawr! (Uses pins 20, 21 for SDA, SCL)
    TWI1->TWI_CWGR = 0;
    TWI1->TWI_CWGR = ((VARIANT_MCK / (2 * 400000)) - 4) * 0x101;
#endif
  }
  if ((reset) && (rst >= 0)) {
    // Setup reset pin direction (used by both SPI and I2C)
    pinMode(rst, OUTPUT);
    digitalWrite(rst, HIGH);
    // VDD (3.3V) goes high at start, lets just chill for a ms
    delay(1);
    // bring reset low
    digitalWrite(rst, LOW);
    // wait 10ms
    delay(10);
    // bring out of reset
    digitalWrite(rst, HIGH);
    // turn on VCC (9V?)
  }

  // Init sequence
  ssd1306_command(SSD1306_DISPLAYOFF);                    // 0xAE
  ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);            // 0xD5
  ssd1306_command(0x80);                                  // the suggested ratio 0x80

  ssd1306_command(SSD1306_SETMULTIPLEX);                  // 0xA8
  ssd1306_command(SSD1306_LCDHEIGHT - 1);

  ssd1306_command(SSD1306_SETDISPLAYOFFSET);              // 0xD3
  ssd1306_command(0x0);                                   // no offset
  ssd1306_command(SSD1306_SETSTARTLINE | 0x0);            // line #0
  ssd1306_command(SSD1306_CHARGEPUMP);                    // 0x8D
  if (vccstate == SSD1306_EXTERNALVCC)
    { ssd1306_command(0x10); }
  else
    { ssd1306_command(0x14); }
  ssd1306_command(SSD1306_MEMORYMODE);                    // 0x20
  ssd1306_command(0x00);                                  // 0x0 act like ks0108
  ssd1306_command(SSD1306_SEGREMAP | 0x1);
  ssd1306_command(SSD1306_COMSCANDEC);

 #if defined SSD1306_128_32
  ssd1306_command(SSD1306_SETCOMPINS);                    // 0xDA
  ssd1306_command(0x02);
  ssd1306_command(SSD1306_SETCONTRAST);                   // 0x81
  ssd1306_command(0x8F);

#elif defined SSD1306_128_64
  ssd1306_command(SSD1306_SETCOMPINS);                    // 0xDA
  ssd1306_command(0x12);
  ssd1306_command(SSD1306_SETCONTRAST);                   // 0x81
  if (vccstate == SSD1306_EXTERNALVCC)
    { ssd1306_command(0x9F); }
  else
    { ssd1306_command(0xCF); }

#elif defined SSD1306_96_16
  ssd1306_command(SSD1306_SETCOMPINS);                    // 0xDA
  ssd1306_command(0x2);   //ada x12
  ssd1306_command(SSD1306_SETCONTRAST);                   // 0x81
  if (vccstate == SSD1306_EXTERNALVCC)
    { ssd1306_command(0x10); }
  else
    { ssd1306_command(0xAF); }

#endif

  ssd1306_command(SSD1306_SETPRECHARGE);                  // 0xd9
  if (vccstate == SSD1306_EXTERNALVCC)
    { ssd1306_command(0x22); }
  else
    { ssd1306_command(0xF1); }
  ssd1306_command(SSD1306_SETVCOMDETECT);                 // 0xDB
  ssd1306_command(0x40);
  ssd1306_command(SSD1306_DISPLAYALLON_RESUME);           // 0xA4
  ssd1306_command(SSD1306_NORMALDISPLAY);                 // 0xA6

  ssd1306_command(SSD1306_DEACTIVATE_SCROLL);

  ssd1306_command(SSD1306_DISPLAYON);//--turn on oled panel
}


void Adafruit_SSD1306::ssd1306_command(uint8_t c) {
  if (sid != -1)
  {
    // SPI
  }
  else
  {
    // I2C
    uint8_t control = 0x00;   // Co = 0, D/C = 0
    _wire.beginTransmission(_i2caddr);
    _wire.write(control);
    _wire.write(c);
    _wire.endTransmission();
  }
}



//***********************************************************************************************
//***********************************************************************************************
//***********************************************************************************************
void Adafruit_SSD1306::DirectPrintChar(int x, int row,unsigned char c)
{
  ssd1306_command(SSD1306_COLUMNADDR);
  ssd1306_command(x);   // Column start address 
  ssd1306_command(x+6); // Column end address 

  ssd1306_command(SSD1306_PAGEADDR);
  ssd1306_command(row); // Page start address (0 = reset)
  
  ssd1306_command(row); // Page end address

   // save I2C bitrate
#ifdef TWBR
    uint8_t twbrbackup = TWBR;
    TWBR = 12; // upgrade to 400KHz!
#endif

    _wire.beginTransmission(_i2caddr);
    _wire.write(0x40);

    for (int i=0;i<5;i++) _wire.write(font[ c * 5 + i]);
    _wire.write(0); //0

    _wire.endTransmission();

#ifdef TWBR
    TWBR = twbrbackup;
#endif
}


void Adafruit_SSD1306::DirectClear()
{
  ssd1306_command(SSD1306_COLUMNADDR);
  ssd1306_command(0);   // Column start address 
  ssd1306_command(SSD1306_LCDWIDTH-1); // Column end address 

  ssd1306_command(SSD1306_PAGEADDR);
  ssd1306_command(0); // Page start address (0 = reset)
  
  ssd1306_command((SSD1306_LCDHEIGHT/8)-1); // Page end address

   // save I2C bitrate
#ifdef TWBR
    uint8_t twbrbackup = TWBR;
    TWBR = 12; // upgrade to 400KHz!
#endif

    for (uint16_t i=0; i<(SSD1306_LCDWIDTH*SSD1306_LCDHEIGHT/(8*16)); i++)
    {
    _wire.beginTransmission(_i2caddr);
    _wire.write(0x40);

    for (int ii=0;ii<16;ii++) _wire.write(0x0);
    _wire.endTransmission();
    }

#ifdef TWBR
    TWBR = twbrbackup;
#endif

}

void Adafruit_SSD1306::DirectPrintString(int x, int row,char *_str)
{
  unsigned char ucLen;
  unsigned char ucColCounter;
  
  for (ucLen=0;ucLen<128;ucLen++) if (_str[ucLen]==0) break;
 
  // analiza dlugosci   
  ssd1306_command(SSD1306_COLUMNADDR);
  ssd1306_command(x);   // Column start address 
  ssd1306_command(x+ (6 * ucLen) ); // Column end address 

  ssd1306_command(SSD1306_PAGEADDR);
  ssd1306_command(row); // Page start address (0 = reset)
  
  ssd1306_command(row); // Page end address

   // save I2C bitrate
#ifdef TWBR
    uint8_t twbrbackup = TWBR;
    TWBR = 12; // upgrade to 400KHz!
#endif
    ucColCounter=0;
    _wire.beginTransmission(_i2caddr);
    _wire.write(0x40);
    for (int istr_char=0; istr_char<ucLen; istr_char++)
    {        
            for (int ii=0;ii<6;ii++) 
            {            
                
                if (ucColCounter==16)
                {
                    _wire.endTransmission();
                    _wire.beginTransmission(_i2caddr);
                    _wire.write(0x40);
                    ucColCounter=0;
                }
                                
                if (ii!=5) _wire.write(font[ _str[istr_char] * 5 + ii]);
                else _wire.write(0); // ostatnia kolumna literty            
                ucColCounter++;
            }            
    }
    _wire.endTransmission();

#ifdef TWBR
    TWBR = twbrbackup;
#endif

}


void Adafruit_SSD1306::DirectVerticalLine(int x, int yHigh, int yLow)
{
  ssd1306_command(SSD1306_MEMORYMODE); // change to vertical mode 
  ssd1306_command(0x1);
   
  ssd1306_command(SSD1306_COLUMNADDR);
  ssd1306_command(x);   // Column start address 
  ssd1306_command(x); // Column end address 

  int iRowHigh = yHigh/8;
  int iRowLow = yLow/8;
  
  ssd1306_command(SSD1306_PAGEADDR);
  ssd1306_command( iRowLow); // Page start address (0 = reset)
  
  ssd1306_command( iRowHigh); // Page end address

   // save I2C bitrate
#ifdef TWBR
    uint8_t twbrbackup = TWBR;
    TWBR = 12; // upgrade to 400KHz!
#endif

    _wire.beginTransmission(_i2caddr);
    _wire.write(0x40);
    
    if (iRowLow==iRowHigh) 
    {
        _wire.write( (0xff >> (iRowLow%8)) & (0xff >> (8-iRowHigh%8)));
    }
   else 
        for (int iDrawRow=iRowLow;iDrawRow<=iRowHigh;iDrawRow++)
   {
        if (iDrawRow==iRowLow) _wire.write(0xff >> (iRowLow%8));
        if (iDrawRow==iRowHigh) _wire.write(0xff >> (8-iRowHigh%8));
        if ((iDrawRow>iRowLow) && (iDrawRow<iRowHigh)) _wire.write(0xff);
   }
   
    _wire.endTransmission();

#ifdef TWBR
    TWBR = twbrbackup;
#endif

  ssd1306_command(SSD1306_MEMORYMODE); // change to Horizontal adresing mode 
  ssd1306_command(0x0);
}

/*
// not ready yet
char Adafruit_SSD1306::DirectReadChar(int x,int row)
{
  ssd1306_command(SSD1306_COLUMNADDR);
  ssd1306_command(x);   // Column start address 
  ssd1306_command(x+16); // Column end address 

  ssd1306_command(SSD1306_PAGEADDR);
  ssd1306_command(row); // Page start address (0 = reset)
  
  ssd1306_command(row); // Page end address

   // save I2C bitrate
#ifdef TWBR
    uint8_t twbrbackup = TWBR;
    TWBR = 12; // upgrade to 400KHz!
#endif

    _wire.beginTransmission(_i2caddr);
    _wire.write(0x40);

    
    for (int i=0;i<16;i++) Serial.print(_wire.read()); 

    _wire.endTransmission();

#ifdef TWBR
    TWBR = twbrbackup;
#endif
Serial.println();
    return 1;
}
*/

void Adafruit_SSD1306::font_bit_reverse()
{ //  fonts reversing for horizontal mirroring effect - necesery for HUD
   unsigned char b;
   for (int i=0;i<(256 * 5);i++)
   {
       b = font[i];
       b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
       b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
       b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
       font[i] = b;
   }
}

