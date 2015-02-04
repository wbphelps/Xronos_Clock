/*
 *********************************************************************************************************
 * HT1632.cpp
 *
 * Apr/10 by FlorinC (http://timewitharduino.blogspot.com/)
 *   Copyrighted and distributed under the terms of the Berkeley license
 *   (copy freely, but include this notice of original authors.)
 *
 * Adapted after HT1632 library by Bill Westfield ("WestfW") (http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1225239439/0);
 *
 * Other contributors:
 *   - fWrite (fast writes) functions courtesy of BroHogan (http://brohogan.blogspot.com/);
 *   - Mark Sproul <MLS> msproul _at_ jove.rutgers.edu
 *
 *********************************************************************************************************
 */

//*********************************************************************************************************
//*	Edit History, started April, 2010
//*	please put your initials and comments here anytime you make changes
//*********************************************************************************************************
//* Apr 15/10 (fc) created file, by restructuring Wise4Sure.pde;
//* Oct 10/10 (rp) ht1632_putBigDigit() amended to allow for multiple fonts
//* Jan 29/11 (fc) adapted to 3216 bi-color display;
//* Jun 12/11 (rp) ht1632_putchar(), ht1632_putSmallChar() amended for ascii character 127 (degree symbol)
//*	Jun 12/11 (rp) ht1632_putBigDigit() now has color and columns parameter;
//* Jun 20/11 (fc) added ht1632_putBitmap(), ht1632_putTinyChar(), setBrightness();
//* Oct 15/11 (rp) added overlayWithSnapshotHorizontal(), overlayWithSnapshotVertical(), ht1632_putLargeChar();
//*					added PUTINSNAPSHOTRAM option plus several small speed optimalizations;
//*
//*********************************************************************************************************


#include "HT1632.h"
#include <avr/pgmspace.h>	// fonts are now loaded in program space;
#include "font3.h"
#include "fontBig.h"
#include "fontSmall.h"
#include "fontTiny.h"
//#include "bitmaps2.h"
//#include "fontLarge.h"



#define CHIP_MAX 4 //Four HT1632Cs on one board
#define CLK_DELAY


extern volatile byte g_GPS_receive;  // flag set to show GPS signal received
extern volatile byte g_IR_receive;  // flag set to show IR signal received


#define HT1632_DATA	14	// Data pin (pin 7 of display connector)
#define HT1632_CS	13	//  Chip Select (pin 1 of display connnector)
#define HT1632_WRCLK	12	// Write clock pin (pin 5 of display connector)
#define HT1632_CLK	15	// clock pin (pin 2 of display connector)

#define MENU_BUTTON_PIN A4// "Menu Button" button on analog 4;
#define SET_BUTTON_PIN A3// "set time" button on digital 17 (analog 3);
#define INC_BUTTON_PIN A2// "inc time" button on digital 16 (analog 2);

//Atmega644/1284 Version of fastWrite - for pins 0-15
#define fWriteA(_pin_, _state_) ( _pin_ < 8 ? (_state_ ? PORTB |= 1 << _pin_ : \
PORTB &= ~(1 << _pin_ )) : (_state_ ? PORTD |= 1 << (_pin_ -8) : PORTD &= ~(1 << (_pin_ -8) )))

//Arduino MEGA compabilte verison of fastWrite for digital pins 0-15
/* #define fWriteA(pin, val) if (val) switch (pin) {     \
    case 0: PORTE |= 1<<0; break;       \
    case 1: PORTE |= 1<<1; break;       \
    case 2: PORTE |= 1<<4; break;       \
    case 3: PORTE |= 1<<5; break;       \
    case 4: PORTG |= 1<<5; break;       \
    case 5: PORTE |= 1<<3; break;       \
    case 6: PORTH |= 1<<3; break;       \
    case 7: PORTH |= 1<<4; break;       \
    case 8: PORTH |= 1<<5; break;       \
    case 9: PORTH |= 1<<6; break;       \
    case 10: PORTB |= 1<<4; break;      \
    case 11: PORTB |= 1<<5; break;      \
    case 12: PORTB |= 1<<6; break;      \
    case 13: PORTB |= 1<<7; break;      \
    } else switch (pin) {               \
    case 0: PORTE &= ~(1<<0); break;    \
    case 1: PORTE &= ~(1<<1); break;    \
    case 2: PORTE &= ~(1<<4); break;    \
    case 3: PORTE &= ~(1<<5); break;    \
    case 4: PORTG &= ~(1<<5); break;    \
    case 5: PORTE &= ~(1<<3); break;    \
    case 6: PORTH &= ~(1<<3); break;    \
    case 7: PORTH &= ~(1<<4); break;    \
    case 8: PORTH &= ~(1<<5); break;    \
    case 9: PORTH &= ~(1<<6); break;    \
    case 10: PORTB &= ~(1<<4); break;   \
    case 11: PORTB &= ~(1<<5); break;   \
    case 12: PORTB &= ~(1<<6); break;   \
    case 13: PORTB &= ~(1<<7); break;   \
    }
/*    
    //Arduino UNO compabilte verison of fastWrite
#define fWriteA(pin, val) if (val) switch (pin) {     \
    case 0: PORTD |= 1<<0; break;       \
    case 1: PORTD |= 1<<1; break;       \
    case 2: PORTD |= 1<<2; break;       \
    case 3: PORTD |= 1<<3; break;       \
    case 4: PORTD |= 1<<4; break;       \
    case 5: PORTD |= 1<<5; break;       \
    case 6: PORTD |= 1<<6; break;       \
    case 7: PORTD |= 1<<7; break;       \
    case 8: PORTB |= 1<<0; break;       \
    case 9: PORTB |= 1<<1; break;       \
    case 10: PORTB |= 1<<2; break;      \
    case 11: PORTB |= 1<<3; break;      \
    case 12: PORTB |= 1<<4; break;      \
    case 13: PORTB |= 1<<5; break;      \
    } else switch (pin) {               \
    case 0: PORTD &= ~(1<<0); break;    \
    case 1: PORTD &= ~(1<<1); break;    \
    case 2: PORTD &= ~(1<<2); break;    \
    case 3: PORTD &= ~(1<<3); break;    \
    case 4: PORTD &= ~(1<<4); break;    \
    case 5: PORTD &= ~(1<<5); break;    \
    case 6: PORTD &= ~(1<<6); break;    \
    case 7: PORTD &= ~(1<<7); break;    \
    case 8: PORTB &= ~(1<<0); break;    \
    case 9: PORTB &= ~(1<<1); break;    \
    case 10: PORTB &= ~(1<<2); break;   \
    case 11: PORTB &= ~(1<<3); break;   \
    case 12: PORTB &= ~(1<<4); break;   \
    case 13: PORTB &= ~(1<<5); break;   \
    }
*/
//Atmega644/1284 Version of fastWrite - for pins 16-31 (Note: PORTA mapping reversed from others)
//#define fWriteA(_pin_, _state_) ( _pin_ < 24 ? (_state_ ? PORTC |= 1 << (_pin_ -16) : \
PORTC &= ~(1 << (_pin_ -16))) : (_state_ ? PORTA |= 1 << (31- _pin_) : PORTA &= ~(1 << (31- _pin_) )))



//*********************************************************************************************************
// our own copy of the "video" memory; 64 bytes for each of the 4 screen quarters;
// each 64-element array maps 2 planes:
// indexes from 0 to 31 are allocated for green plane;
// indexes from 32 to 63 are allocated for red plane;
// when a bit is 1 in both planes, it is displayed as orange (green + red);
byte ht1632_shadowram[64][4] = {0};




//**************************************************************************************************
//Function Name: OutputCLK_Pulse
//Function Feature: enable CLK_74164 pin to output a clock pulse
//Input Argument: void
//Output Argument: void
//**************************************************************************************************
void OutputCLK_Pulse(void) //Output a clock pulse
{
	fWriteA(HT1632_CLK, HIGH);
	//  digitalWrite(HT1632_CLK, HIGH);
	fWriteA(HT1632_CLK, LOW);
	//  digitalWrite(HT1632_CLK, LOW);
}


//**************************************************************************************************
//Function Name: OutputA_74164
//Function Feature: enable pin A of 74164 to output 0 or 1
//Input Argument: x: if x=1, 74164 outputs high. If x?1, 74164 outputs low.
//Output Argument: void
//**************************************************************************************************
void OutputA_74164(unsigned char x) //Input a digital level to 74164
{
	fWriteA(HT1632_CS, (x==1 ? HIGH : LOW));
//    digitalWrite(HT1632_CS, (x==1 ? HIGH : LOW));
}


//**************************************************************************************************
//Function Name: ChipSelect
//Function Feature: enable HT1632C
//Input Argument: select: HT1632C to be selected
// If select=0, select none.
// If s<0, select all.
//Output Argument: void
//**************************************************************************************************
void ChipSelect(int select)
{
  unsigned char tmp = 0;
  if(select<0) //Enable all HT1632Cs
  {
    OutputA_74164(0);
    CLK_DELAY;
    for(tmp=0; tmp<CHIP_MAX; tmp++)
    {
      OutputCLK_Pulse();
    }
  }
  else if(select==0) //Disable all HT1632Cs
  {
    OutputA_74164(1);
    CLK_DELAY;
    for(tmp=0; tmp<CHIP_MAX; tmp++)
    {
      OutputCLK_Pulse();
    }
  }
  else
  {
    OutputA_74164(1);
    CLK_DELAY;
    for(tmp=0; tmp<CHIP_MAX; tmp++)
    {
      OutputCLK_Pulse();
    }
    OutputA_74164(0);
    CLK_DELAY;
    OutputCLK_Pulse();
    CLK_DELAY;
    OutputA_74164(1);
    CLK_DELAY;
    tmp = 1;
    for( ; tmp<select; tmp++)
    {
      OutputCLK_Pulse();
    }
  }
}


//*********************************************************************************************************

void ht1632_setup()
{
  pinMode(HT1632_CS, OUTPUT);
  digitalWrite(HT1632_CS, HIGH); 	/* unselect (active low) */
  pinMode(HT1632_WRCLK, OUTPUT);
  pinMode(HT1632_DATA, OUTPUT);
  pinMode(HT1632_CLK, OUTPUT);

  for (byte j=1; j<5; j++)
  {
    ht1632_sendcmd(j, HT1632_CMD_SYSDIS);  // Disable system
    ht1632_sendcmd(j, HT1632_CMD_COMS00);
    ht1632_sendcmd(j, HT1632_CMD_MSTMD); 	/* Master Mode */
    ht1632_sendcmd(j, HT1632_CMD_RCCLK);  // HT1632C
    ht1632_sendcmd(j, HT1632_CMD_SYSON); 	/* System on */
    ht1632_sendcmd(j, HT1632_CMD_LEDON); 	/* LEDs on */
  }
 
  ht1632_clear();
  delay(100);
}


//*********************************************************************************************************
/*
 * plot a point on the display, with the upper left hand corner
 * being (0,0), and the lower right hand corner being (31, 15);
 * parameter "color" could have one of the 4 values:
 * black (off), red, green or yellow;
 */
void ht1632_plot (int x, int y, byte color)
{
  if (x<0 || x>=X_MAX || y<0 || y>=Y_MAX)
    return;
  if (color & PUTINSNAPSHOTRAM)
  {
	put_snapshotram(x, y, color & ORANGE);
	return;
  }
  
// if (color != BLACK && color != GREEN && color != RED && color != ORANGE)
  if (color > ORANGE)
    return;
  
//  if (color == get_shadowram(x, y))
//    return;		// nothing to change;

  byte nChip = x / 16 + (y > 7 ? 2 : 0) ;
  y = y % 8;
  byte addr = ((x % 16) << 1) + (y >> 2);
  byte bitval = 8 >> (y & 3);  								// compute which bit will need set;
  
  byte val = (ht1632_shadowram[addr][nChip] & bitval) ? 1 : 0;
  val +=  (ht1632_shadowram[addr+32][nChip] & bitval) ? 2 : 0;

  if (color == val)
    return;		// nothing to change;
	
  switch (color)
  {
    case BLACK:
      // clear the bit in both planes;
      ht1632_shadowram[addr][nChip] &= ~bitval;
      ht1632_senddata(nChip+1, addr, ht1632_shadowram[addr][nChip]);
      addr = addr + 32;
      ht1632_shadowram[addr][nChip] &= ~bitval;
      ht1632_senddata(nChip+1, addr, ht1632_shadowram[addr][nChip]);
      break;
    case GREEN:
      // set the bit in the green plane and clear the bit in the red plane;
      ht1632_shadowram[addr][nChip] |= bitval;
      ht1632_senddata(nChip+1, addr, ht1632_shadowram[addr][nChip]);
      addr = addr + 32;
      ht1632_shadowram[addr][nChip] &= ~bitval;
      ht1632_senddata(nChip+1, addr, ht1632_shadowram[addr][nChip]);
      break;
    case RED:
      // clear the bit in green plane and set the bit in the red plane;
      ht1632_shadowram[addr][nChip] &= ~bitval;
      ht1632_senddata(nChip+1, addr, ht1632_shadowram[addr][nChip]);
      addr = addr + 32;
      ht1632_shadowram[addr][nChip] |= bitval;
      ht1632_senddata(nChip+1, addr, ht1632_shadowram[addr][nChip]);
      break;
    case ORANGE:
      // set the bit in both the green and red planes;
      ht1632_shadowram[addr][nChip] |= bitval;
      ht1632_senddata(nChip+1, addr, ht1632_shadowram[addr][nChip]);
      addr = addr + 32;
      ht1632_shadowram[addr][nChip] |= bitval;
      ht1632_senddata(nChip+1, addr, ht1632_shadowram[addr][nChip]);
      break;
  }
}


//*********************************************************************************************************
/*
 * ht1632_clear
 * clear the display, and the shadow memory, and the snapshot
 * memory.  This uses the "write multiple words" capability of
 * the chipset by writing all 96 words of memory without raising
 * the chipselect signal.
 */
void ht1632_clear()
{
  // clear our own shadow memory;
  for (byte i=0; i < 4; i++)
  {
    for (byte j=0; j < 64; j++)
      ht1632_shadowram[j][i] = 0;
  }

  // clear the display memory;
  for (byte i=0; i<96; i++)
  {
    ht1632_senddata(1, i, 0);  // clear the display!
    ht1632_senddata(2, i, 0);  // clear the display!
    ht1632_senddata(3, i, 0);  // clear the display!
    ht1632_senddata(4, i, 0);  // clear the display!
  }
  
  // turn signal LED indicators back on if they were on (wbp)
  if (g_GPS_receive == 1)
    plot(31,1,GREEN); //Plot Indicator dot (GPS signal was received)
  else if (g_GPS_receive == 2)
    plot(31,1,ORANGE); //Plot Indicator dot (GPS signal was received)
  else if (g_GPS_receive == 3)
    plot(31,1,RED); //Plot Indicator dot (GPS signal was received)

  if (g_IR_receive == 1)
    plot(31,2,GREEN); //Plot Indicator dot (IR signal was received)
  
}


//*********************************************************************************************************
/*
 * snapshot_shadowram
 * Copy the shadow ram into the snapshot ram (the upper bits)
 * This gives us a separate copy so we can plot new data while
 * still having a copy of the old data.  snapshotram is NOT
 * updated by the plot functions (except "clear").
 */
void snapshot_shadowram()
{
  for (byte nQuarter=0; nQuarter<4; nQuarter++)
  {
    for (byte i=0; i<64; i++)
    {
      // copy the video bits (lower 4) in the upper 4;
      byte val = ht1632_shadowram[i][nQuarter];
      ht1632_shadowram[i][nQuarter] = (val & 0x0F) + (val << 4);
    }
  }
}


//*********************************************************************************************************
/*
 * return the value of a pixel from the video memory (either BLACK, RED, GREEN, ORANGE);
 */
byte get_shadowram(byte x, byte y)
{
	return get_videoram(x, y, 0x08);					// get lower 4 bit;
}	


//*********************************************************************************************************
/*
 * get_snapshotram
 * get a pixel value from the snapshot ram instead of the actual video memory;
 * return BLACK, GREEN, RED or ORANGE;
 */
byte get_snapshotram(byte x, byte y)
{
	return get_videoram(x, y, 0x80);				// get higher 4 bit;
}


//*********************************************************************************************************
/*
 * return the value of a pixel from the video memory (either BLACK, RED, GREEN, ORANGE);
 */
byte get_videoram(byte x, byte y, byte whichBit)
{
   byte nQuarter = x / 16 + (y > 7 ? 2 : 0);
   y = y % 8;
   byte addr = ((x % 16) << 1) + (y >> 2);
   byte bitval = whichBit >> (y & 3);
   byte val = (ht1632_shadowram[addr][nQuarter] & bitval) ? 1 : 0;
   val += (ht1632_shadowram[addr+32][nQuarter] & bitval) ? 2 : 0;
   return val;
}


//*********************************************************************************************************
/*
 * write the value of a pixel in the snapshot memory (either BLACK, RED, GREEN, ORANGE);
 */
void put_snapshotram(byte x, byte y, byte color)
{
   byte nChip = x / 16 + (y > 7 ? 2 : 0);
   y = y % 8;
   byte addr = ((x % 16) << 1) + (y >> 2);
   byte bitval = 0x80 >> (y & 3);

   switch (color)
   {
    case BLACK:
      // clear the bit in both planes;
      ht1632_shadowram[addr][nChip] &= ~bitval;
      ht1632_shadowram[addr+32][nChip] &= ~bitval;
      break;
    case GREEN:
      // set the bit in the green plane and clear the bit in the red plane;
      ht1632_shadowram[addr][nChip] |= bitval;
      ht1632_shadowram[addr+32][nChip] &= ~bitval;
      break;
    case RED:
      // clear the bit in green plane and set the bit in the red plane;
      ht1632_shadowram[addr][nChip] &= ~bitval;
      ht1632_shadowram[addr+32][nChip] |= bitval;
      break;
    case ORANGE:
      // set the bit in both the green and red planes;
      ht1632_shadowram[addr][nChip] |= bitval;
      ht1632_shadowram[addr+32][nChip] |= bitval;
      break;
   }  
}


//*********************************************************************************************************
/*
 * overlay the current display with the snapshot memory line by line from top to bottom;
 */
void overlayWithSnapshotHorizontal(boolean clearSnapshotRam, int msec)
{
	for (byte y = 0; y < 16; ++y)
	{
		for (byte x = 0; x < 32; ++x)
			ht1632_plot(x, y, get_videoram(x, y, 0x80));
		delay(msec);
	}
	
	if (clearSnapshotRam == true)
	{
		for (byte i=0; i < 4; i++)
		{
			for (byte j=0; j < 64; j++)
				ht1632_shadowram[j][i] &= 0x0F;
		}
	}
}


//*********************************************************************************************************
/*
 * overlay the current display with the snapshot memory column by column from left to right;
 */
void overlayWithSnapshotVertical(boolean clearSnapshotRam, int msec)
{
	for (byte x = 0; x < 32; ++x)
	{
		for (byte y = 0; y < 16; ++y)
			ht1632_plot(x, y, get_videoram(x, y, 0x80));
		delay(msec);
	}	

	if (clearSnapshotRam == true)
	{
		for (byte i=0; i < 4; i++)
		{
			for (byte j=0; j < 64; j++)
				ht1632_shadowram[j][i] &= 0x0F;
		}
	}
}


//*********************************************************************************************************
/*
 * ht1632_writebits
 * Write bits (up to 8) to h1632 on pins ht1632_data, ht1632_wrclk
 * Chip is assumed to already be chip-selected
 * Bits are shifted out from MSB to LSB, with the first bit sent
 * being (bits & firstbit), shifted till firsbit is zero.
 */
void ht1632_writebits (byte bits, byte firstbit)
{
	while (firstbit)
	{
		fWriteA(HT1632_WRCLK, LOW);	// digitalWrite(HT1632_WRCLK, LOW);

		if (bits & firstbit)
		{
			fWriteA(HT1632_DATA, HIGH);	// digitalWrite(HT1632_DATA, HIGH);
		} 
		else
		{
			fWriteA(HT1632_DATA, LOW);	// digitalWrite(HT1632_DATA, LOW);
		}

		fWriteA(HT1632_WRCLK, HIGH);	// digitalWrite(HT1632_WRCLK, HIGH);
		firstbit >>= 1;
	}
}


//*********************************************************************************************************
/*
 * ht1632_sendcmd
 * Send a command to the ht1632 chip.
 */
void ht1632_sendcmd (byte chipNo, byte command)
{
  ChipSelect(chipNo);
  ht1632_writebits(HT1632_ID_CMD, 1<<2);  // send 3 bits of id: COMMMAND
  ht1632_writebits(command, 1<<7);  // send the actual command
  ht1632_writebits(0, 1); 	/* one extra dont-care bit in commands. */
  ChipSelect(0);
}



//*********************************************************************************************************
/*
 * ht1632_senddata
 * send a nibble (4 bits) of data to a particular memory location of the
 * ht1632.  The command has 3 bit ID, 7 bits of address, and 4 bits of data.
 *    Select 1 0 1 A6 A5 A4 A3 A2 A1 A0 D0 D1 D2 D3 Free
 * Note that the address is sent MSB first, while the data is sent LSB first!
 * This means that somewhere a bit reversal will have to be done to get
 * zero-based addressing of words and dots within words.
 */
void ht1632_senddata (byte chipNo, byte address, byte data)
{
  ChipSelect(chipNo);
  ht1632_writebits(HT1632_ID_WR, 1<<2);  // send ID: WRITE to RAM
  ht1632_writebits(address, 1<<6); // Send address
  ht1632_writebits(data, 1<<3); // send 4 bits of data
  ChipSelect(0);
}




//*********************************************************************************************************
/*
 * Copy a character glyph from the myfont data structure to
 * display memory, with its upper left at the given coordinate
 * This is unoptimized and simply uses plot() to draw each dot.
 * (fc, Jan 30/2011) display character using the specified color;
 */
void ht1632_putchar(int x, int y, char c, byte color)
{
	// fonts defined for ascii 32 and beyond (index 0 in font array is ascii 32);
	byte charIndex;

	// replace undisplayable characters with blank;
	if (c < 32 || c > 127)
	{
		charIndex	=	0;
	}
	else
	{
		charIndex	=	c - 32;
	}

	// move character definition, pixel by pixel, onto the display;
	// fonts are defined as one byte per row;
	for (byte row=0; row<8; row++)
	{
		byte rowDots	=	pgm_read_byte_near(&myfont[charIndex][row]);
		for (byte col=0; col<6; col++)
		{
			if (rowDots & (1<<(5-col)))
				ht1632_plot(x+col, y+row, color);
			else 
				ht1632_plot(x+col, y+row, color & PUTINSNAPSHOTRAM);
		}
	}
}


//*********************************************************************************************************
/*
 * Copy a character glyph from the smallFont data structure to
 * display memory, with its upper left at the given coordinate
 * This is unoptimized and simply uses plot() to draw each dot.
 */
void ht1632_putSmallChar(int x, int y, char c, byte color)
{
	// fonts defined for ascii 32 and beyond (index 0 in font array is ascii 32);
	byte charIndex;

	// replace undisplayable characters with blank;
	if (c < 32 || c > 127)
	{
		charIndex	=	0;
	}
	else
	{
		charIndex	=	c - 32;
	}

	// move character definition, pixel by pixel, onto the display;
	// fonts are defined as one byte per row;
	for (byte row=0; row<8; row++)
	{
		byte rowDots	=	pgm_read_byte_near(&smallFont[charIndex][row]);
		for (byte col=0; col<6; col++)
		{
			if (rowDots & (1<<(5-col)))
				ht1632_plot(x+col, y+row, color);
			else 
				ht1632_plot(x+col, y+row, color & PUTINSNAPSHOTRAM);
		}
	}
}


//*********************************************************************************************************
/*
 * Copy a character glyph from the proportional, largeFont data structure to
 * display memory, with its upper left at the given coordinate.
 * Font is 14 dots high and max 11 dots width.
 * This is unoptimized and simply uses plot() to draw each dot.
 
int ht1632_putLargeChar(int x, int y, char c, byte color)
{
	// fonts defined for ascii 32 and beyond (index 0 in font array is ascii 32);
	byte charIndex, col, row;
	
	// replace undisplayable characters with blank;
	if (c < 32 || c > 127)
	{
		charIndex	=	0;
	}
	else
	{
		charIndex	=	c - 32;
	}

	// move character definition, pixel by pixel, onto the display;
	// Fonts are defined as up to 14 bit per row and max. 11 columns;
	// first row is always zero to create the space between the characters;
	
	for (col=0; col < 11; ++col)					// max 11 columns;
	{
		uint16_t dots = pgm_read_word_near(&largeFont[charIndex][col]);
		if (dots == 0) 								// stop if all bits zero;
			break;
	
		for (row=0; row < 14; row++) 
		{
			if (dots & (0x4000 >> row))    			// max 14 rows;
				plot(x+col, y+row, color);
			else 
				plot(x+col, y+row, color & PUTINSNAPSHOTRAM);
		}
	}
	return x+col;
}
*/

//*********************************************************************************************************
void displayStaticLine(char* text, byte y, byte color)
{
	// try to center the text;
	int len	=	strlen(text);
	int nx	=	(X_MAX+1 - 6*len) / 2;
	if (nx < 0)
	{
		nx = 0;		// text too long to fit on the screen;
	}
	for (int i=0; i<len; i++)
	{
		ht1632_putSmallChar(nx + i*6, y, text[i], color);
	}
}


//*********************************************************************************************************
/* (fc, Aug 1/10)
 * display a big digit, defined on a 6x12 grid;
 * the purpose is to display the time H12:MM on the 24x16 matrix;
 */
// (rp, Oct 2010) modified to use multiple big fonts;
void ht1632_putBigDigit(int x, int y, int digit, int fontNbr, byte color, int columns)
{
	// move character definition, pixel by pixel, onto the display;
	// a big digit is defined as 12 rows, one byte per row;

    int fontOffset = fontNbr * CHARS_IN_FONT;

	for (byte row=0; row < BYTES_PER_CHARS; row++)
	{
		byte rowDots = pgm_read_byte_near(&bigFont[digit + fontOffset][row]);
		for (byte col=0; col<columns; col++)
		{
			if (rowDots & (1<<((columns - 1) - col)))
				ht1632_plot(x+col, y+row, color);
			else 
				ht1632_plot(x+col, y+row, color & PUTINSNAPSHOTRAM);
		}
	}
}

//*********************************************************************************************************
/* (fc, Jun 10/2011)
 * load and display a given bitmap (defined in bitmaps.h);
 
void ht1632_putBitmap(int x, int y, byte indexBmp, byte color)
{
  if (color)
	// move character definition, pixel by pixel, onto the display;
	// bitmap has 14 rows x 14 columns, one WORD per row;

	for (byte row=0; row < 14; row++)
	{
		uint16_t rowDots = pgm_read_word_near(&bitmap[indexBmp][row]);
		for (byte col=0; col<14; col++)
		{
			if (rowDots & (1<<(13-col)))
				ht1632_plot(x+col, y+row, color);
			else
				// little hack for pacman mode; comment for any other bitmap;
				if (col > 0 || (row!=6 && row!=7))
					ht1632_plot(x+col, y+row, color & PUTINSNAPSHOTRAM);
		}
	}
}

*/
//*********************************************************************************************************
/*
 * Copy a character glyph from the tinyFont data structure to
 * display memory, with its upper left at the given coordinate
 * This is unoptimized and simply uses plot() to draw each dot.
*/
void ht1632_putTinyChar(int x, int y, char c, byte color)
{
	// fonts defined for ascii 32 and beyond (index 0 in font array is ascii 32);
	byte charIndex;

	// replace undisplayable characters with blank;
	if (c < 32 || c > 126)
	{
		charIndex	=	0;
	}
	else
	{
		charIndex	=	c - 32;
	}

	// move character definition, pixel by pixel, onto the display;
	// fonts are defined as one byte per row;
	for (byte row=0; row<8; row++)
	{
		byte rowDots	=	pgm_read_byte_near(&tinyFont[charIndex][row]);
		for (byte col=0; col<4; col++)
		{
			if (rowDots & (1<<(3-col)))
				ht1632_plot(x+col, y+row, color);
			else 
				ht1632_plot(x+col, y+row, color & PUTINSNAPSHOTRAM);
		}
	}
}


//*********************************************************************************************************
/*
 * Display a string with the tiny font at the given coordinates.
*/
void ht1632_putTinyString(int x, int y, const char* str, byte color)
{
  for (int i=0; i<strlen(str); i++)
  {
    // start one column to the left (the first column of each character is always empty);
    ht1632_putTinyChar(x+i*4-1, y, str[i], color);
  }
}


// =======================================================================================
// ---- Sets LED brightness ----
// ---- Setting of 1 = brightness of one, rest is x3 up to 5
// doc for HT1632 says PWM values are 0 to 15 (wbp)
// PWM relationship to brightness is not linear
// ---------------------------------------------------------------------------------------
void setBrightness(byte nLevel)
{
//  byte multi; // Multiplyer
//  if (nLevel == 1) multi=1; // Make sure we set lowest setting
//  else multi=3; // Do brightness in 3x increments
  byte levels[] = {0, 2, 4, 8, 15};
  if (nLevel > 5)
      nLevel = 5;

  for (byte i=1; i<=4; i++)
//    ht1632_sendcmd(i, HT1632_CMD_PWM + (nLevel-1)*multi);
    ht1632_sendcmd(i, HT1632_CMD_PWM + levels[nLevel-1]);  // wbp
  
}

/***********************************************************************
 * Scrolling  fumctions 
 * for scrolling text and bitmaps
 * Please take only fonts with fixed heigth and width, otherwise the
 * chars will be overlapping
 * Original functions by Bill Ho
 * modified by LensDigital (added interrupt by buttons)
 ***********************************************************************/
boolean scrolltextsizexcolor(int y, char Str1[ ], byte color, int delaytime){
   int messageLength = strlen(Str1)+ 1;
   byte showcolor,showsecondcolor;
  int xa = 0;
  while (xa<1) {
    int xpos = X_MAX;
    while (xpos > (-1 * ( 7 * messageLength))) {  // wbp: 7 bytes per char instead of 8
      for (int i = 0; i < messageLength; i++) {
        if (color==4) color=random(3)+1;
        ht1632_putchar(xpos + (7 * i),  y, Str1[i], color);  // wbp: 7 instead of 8
      }
      if ( (digitalRead(SET_BUTTON_PIN) == HIGH) || (digitalRead(MENU_BUTTON_PIN) == HIGH) || (digitalRead(INC_BUTTON_PIN) == HIGH)  ) {cls (); return false; } // Interrupt
      delay(delaytime);// reduce speed of scroll
      xpos--;
    }
    xa =1;
  }
  return true; // wasn't interrupted
}

// =======================================================================================
// ---- Display single digit at requested location in requested color ----
// By: Bratan
// ---------------------------------------------------------------------------------------
void showDigit(int x, int y, int digit, int fontW, byte fontNum,byte color){
  int myOffset=7;// For char/digits font 8 px wide
  int fontOffset = fontNum * CHARS_IN_FONT;
  fontW=fontW-1; // Font width -1
  // Font height
  for (int row=0; row<BYTES_PER_CHARS; row++){
   for (int i=fontW;i>=0;i--){
    if (pgm_read_byte_near(&bigFont[digit+fontOffset][row]) & (1<<i) ) { plot ( (myOffset+x)-i,row+y,color); } 
    else { plot ( (myOffset+x)-i,row+y,BLACK ); }
   }
  }
}

// =======================================================================================
// ---- Display Static text at specified location ----
// By: Bratan
// parameters: x - starting position, y - vertical position
//             myString - array of chars, 
//             fntsz - Font Size. 1-Normal, 2-Small, 3-Tiny
//             color -  RED, GREEEN, ORANGE, BLACK or RANDOM
// ---------------------------------------------------------------------------------------
void showText(int x, int y, char * myString, int fntsz, byte color){
  if (color==4)color=random(3)+1; // Select random color
  for (int i=0;i< (strlen(myString));i++){
     switch (fntsz)
    {
     case 1: // Normal
     ht1632_putchar(i*6+x,y,myString[i],color); // i*8 mutliplies position by char width "8"
     break;
     case 2: // Small
     ht1632_putSmallChar(x+i*6,y,myString[i],color); // i*8 mutliplies position by char width "8"
     break;
     case 3: // Tiny 
     ht1632_putTinyChar(x+i*4,y,myString[i],color); // i*8 mutliplies position by char width "8"
     break;
     default: // Normal (default)
     ht1632_putchar(i*6+x,y,myString[i],color); // i*8 mutliplies position by char width "8"
    }
    
   }
}
