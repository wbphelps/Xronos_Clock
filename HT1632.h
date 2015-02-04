/*
 *********************************************************************************************************
 * HT1632.h - defintions for Holtek HT1632 LED driver.
 *
 * Apr/10 by FlorinC (http://timewitharduino.blogspot.com/)
 *   Copyrighted and distributed under the terms of the Berkeley license
 *   (copy freely, but include this notice of original authors.)
 *
 * Adapted after HT1632 library by Bill Westfield ("WestfW") (http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1225239439/0);
 *
 *********************************************************************************************************
 */

//*********************************************************************************************************
//*	Edit History, started April, 2010
//*	please put your initials and comments here anytime you make changes
//*********************************************************************************************************
//* Apr 15/10 (fc) created file, by restructuring Wise4Sure.pde;
//* Oct 10/10 (rp) adapted ht1632_putBigDigit() for multiple fonts;
//* Jan 29/11 (fc) modified for Wise Clock 3 (3216 bicolor display);
//* Jun 20/11 (rp) added color and columns parameter to putBigDigit(); 
//* Jun 20/11 (fc) added setBrightness(), ht1632_plot() now uses int instead of byte; 
//* Oct 15/11 (rp) added PUTINSNAPSHOTRAM, overlayWithSnapshotHorizontal(), overlayWithSnapshotVertical();
//*
//*********************************************************************************************************


#ifndef _HT1632_
#define _HT1632_


#if ARDUINO < 100
  #include <WProgram.h>
#else
  #include <Arduino.h>
#endif


#define X_MAX 32
#define Y_MAX 16

// possible values for a pixel;
#define BLACK  0
#define RED    1
#define GREEN  2
#define ORANGE 3
#define RANDOMCOLOR 4 // random color for the whole object

#define PUTINSNAPSHOTRAM 0x10			// if set write to snapshot memory instead of shadow ram.;
#define CLEARSNAPSHOTRAM true			// used in overlay functions;
#define NOCLEARSNAPSHOTRAM false
#define LEDBrightness 8 // Set Brightness of teh display from 1-15

#define plot(x,y,v)  ht1632_plot(x,y,v)
#define cls          ht1632_clear



/*
 * commands written to the chip consist of a 3 bit "ID", followed by
 * either 9 bits of "Command code" or 7 bits of address + 4 bits of data.
 */
#define HT1632_ID_CMD 4		/* ID = 100 - Commands */
#define HT1632_ID_RD  6		/* ID = 110 - Read RAM */
#define HT1632_ID_WR  5		/* ID = 101 - Write RAM */
#define HT1632_ID_BITS (1<<2)   /* IDs are 3 bits */

#define HT1632_CMD_SYSDIS 0x00	/* CMD= 0000-0000-x Turn off oscil */
#define HT1632_CMD_SYSON  0x01	/* CMD= 0000-0001-x Enable system oscil */
#define HT1632_CMD_LEDOFF 0x02	/* CMD= 0000-0010-x LED duty cycle gen off */
#define HT1632_CMD_LEDON  0x03	/* CMD= 0000-0011-x LEDs ON */
#define HT1632_CMD_BLOFF  0x08	/* CMD= 0000-1000-x Blink ON */
#define HT1632_CMD_BLON   0x09	/* CMD= 0000-1001-x Blink Off */
#define HT1632_CMD_SLVMD  0x10	/* CMD= 0001-00xx-x Slave Mode */
#define HT1632_CMD_MSTMD  0x14	/* CMD= 0001-01xx-x Master Mode */
#define HT1632_CMD_RCCLK  0x18	/* CMD= 0001-10xx-x Use on-chip clock */
#define HT1632_CMD_EXTCLK 0x1C	/* CMD= 0001-11xx-x Use external clock */
#define HT1632_CMD_COMS00 0x20	/* CMD= 0010-ABxx-x commons options */
#define HT1632_CMD_COMS01 0x24	/* CMD= 0010-ABxx-x commons options */
#define HT1632_CMD_COMS10 0x28	/* CMD= 0010-ABxx-x commons options */
#define HT1632_CMD_COMS11 0x2C	/* CMD= 0010-ABxx-x commons options */
#define HT1632_CMD_PWM    0xA0	/* CMD= 101x-PPPP-x PWM duty cycle */
#define HT1632_CMD_BITS (1<<7)



extern byte ht1632_shadowram[64][4];		// our copy of the display's RAM


void		snapshot_shadowram();
byte		get_snapshotram(byte x, byte y);
byte		get_shadowram(byte x, byte y);
byte 		get_videoram(byte x, byte y, byte which4Bits);
void 		put_snapshotram(byte x, byte y, byte color);
void 		overlayWithSnapshotHorizontal(boolean clearSnapshotRam, int msec);
void 		overlayWithSnapshotVertical(boolean clearSnapshotRam, int msec);

void		ht1632_setup();
void		ht1632_clear();

void		ht1632_writebits(byte bits, byte firstbit);
void		ht1632_sendcmd(byte chipNo, byte command);
void		ht1632_senddata(byte chipNo, byte address, byte data);
void		ht1632_plot	       (int x, int y, byte color);
void		ht1632_putchar     (int x, int y, char c, byte color);
void		ht1632_putSmallChar(int x, int y, char c, byte color);
void		ht1632_putTinyChar (int x, int y, char c, byte color);
//int 		ht1632_putLargeChar(int x, int y, char c, byte color);
//void		ht1632_putBigDigit (int x, int y, int digit, int fontNbr, byte color, int columns);
//void		ht1632_putBitmap   (int x, int y, byte indexBmp, byte color=ORANGE);
void		ht1632_putTinyString(int x, int y, const char* str, byte color);

void		displayStaticLine(char* text, byte y, byte color);
void		setBrightness(byte level);
void            ht1632_putcharsizecolor(int x, int y,unsigned char c,  char size, byte color, byte secondcolor,  int columncountfont, char rowcountfont, char oddeven);
boolean            scrolltextsizexcolor(int y,char Str1[ ], byte color, int delaytime);
void            showDigit(int x, int y, int digit, int fontW, byte fontNum, byte color);
void            showText(int x, int y, char * myString, int fntsz, byte color);

#endif  // _HT1632_

