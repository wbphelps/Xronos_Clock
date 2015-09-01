//***********************************************************************
#define FIRMWARE_VER 242 // Current Firmware version (wbp)
/*
* December 2014 - April 2015 - mods by William Phelps (wm@usa.net)
* Ver 2.42 (08/28/2015)
*
* CHANGES & BUG FIXES (Most recent first)
* fix Info display bug & defaults
* move TZ to Settings
* smaller increments for photocell max
* add setting for TZ Hour & Minute
* add support for DHT22 temp/humidity sensor
* progressive alarm volume indicator
* Set button becomes Decrement when adjusting time, date, etc.
* progressive alarm: adjust vol at preset intervals; fix alarm skip bug
* change auto bright Hi limits (100, 500)
* Auto DST working, fix some typos, move DSTmode & DSToffset to Settings
* add Startup Quiet option
* set alarm vol when starting alarm
* add #define for Radio, IR, GPS - conditional compile
* fix alarm volume display (& rework volume code)
* Menu button exits QMenu
* increase # of alarm tones to 12, use define
* add settable alarm snooze time 1 to 15 minutes
* hold Alarm Reset to skip upcoming alarm
* add "alarm reset" scroll message
* rewrite button logic, merge button checks to common routine
* global blink timers, bug fixes
* option to enable/disable blinking colon
* add tick sound for setting time
* add button debounce/hold/repeat 
* fix debounce timer, hold button to reset alarm
* fix green/red color bug
* fix temp<20 bug in sayTemp(); (from Len)
* use structure for settings saved in EE
* add display & set for photocell levels (min, max), BrtLo (0 to 100 by 10) and BrtHi (200 to 750 by 50)
* flip alarm LED colors - green=alarm in next 24 hours, red if not
* atomic (single fetch) time/date
* set alarm LED color if alarm is due within next 24 hours
* add progressive alarm volume on/off option for alarm setting
* blink alarm indicators if playing or snoozing
* change menu timeout to 5 seconds
* move alarm indicators to edges
* auto color, depending on light level
* adjust some menu spacings
* set IR & GPS status LED's after clear
* small changes to fonts 3 & 4
* switch IRRemote to use Timer3 interrupt instead of Timer2
* Add GPS On/Off in System settings
* AUTO DST working!
* DST setting: Off, On, Auto
* fix bug in brightness averaging
* audio: if 12hr mode & minute == 0, don't say "hundred"
* change colon display & offset
* GPS support
* "Temp:" (no In/Out) if no radio
* "Temp In:" instead of "In Temp"
* PM LED instead of AM
* auto bright - adjust at 1 second intervals (was 10)
* auto bright - average last 3 readings to smooth out changes
* bugfix: brightness set to auto by error
* logarithmic brightness levels
*
*** TODO ***
* Add TZ Hr & TZ Mn to settings?
* Add night/quiet modes
***********************************************************************/
/***********************************************************************
* July 11, 2013 LensDigital 
* Ver 2.09 (10/28/2014): 
*                      - Feature: Added DST +/- 1 hour function with menu item. 
*                      - Commented out not needed definition(s). 
*                      - Added MAX_MENUS definition for menu system
*                      - Added MAX_SUBMENUS definition for submenu system. Also decreased elements of subMenu[] array to 8 (from 9)
* See www.xronosclock.com for project details, schematics, etc.
* NOTE: By default RFM12B is disabled. If you have one onboard, chnage RFM12B_Enabled=false to RFM12B_Enabled=true, upload. Then change setting in System Menu (Set RFM12 to YES) and reboot clock).
* NOTE: With IRRemote library ATMega644p based clock doesn't have enough RAM to run. Comment out all IR related code to run on ATmega644p chip.
**********************************************************************/
/***********************************************************************
 * Scrolling  fumction by Bill Ho 
 * adapted by roomtek http://code.google.com/p/sure-green-matrix/
 *   Copyrighted and distributed under the terms of the Berkely license
 *   (copy freely, but include this notice of original author.)
 ***********************************************************************/

// Which temperature sensor?
//#define DS18B20  // DS18B20 one wire temperature sensor
#define DHT22  // DHT22 one wire temperature & humidity sensor

#include <Wire.h>
#include "HT1632.h"
//#include <avr/pgmspace.h> - not used
#include <Time.h>  
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t
#include <stdlib.h> // Used for string manipulations and string to int conversions
#include "settings.h" // settings & options stored in EE memory
#include "WaveUtil.h" // Used by wave shield
#include "WaveHC.h" // Used by wave shield (library modified by LensDigital to accomodate ATMega644p/ATMega1284p)
//#include <EEPROM.h> - not used, see settings.h, settings.ino
#if defined  DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>
#elif defined DHT22
#include "DHT.h"
#endif

// EE version - change this to force reset of EE memory
#define EE_VERSION 13

//#define PRT_DEBUG
//#define PRT_ERROR
//#define BTN_DEBUG  // for button debugging?
//#define DST_DEBUG
//#define DBG_ALARM

// ============================================================================================
// Important User Hardware config settings, modify as needed
// ============================================================================================
//#define RFM12B_PRESENT  // Defines if RFM12B Chip present.  Set to true to enable. Must also have ATMega1284p! Will not work with ATMega644p chip
#define IR_PRESENT  // Set to True if IR receiver is present. Must also have ATMega1284p! Will not work with ATMega644p chip
#define GPS_PRESENT // Set to True if GPS receiver is present
#define AUTO_BRIGHTNESS_ON 0  //Set to 1 to disable autobrightness menu feature, 0 to enable if photocell is present.
// ============================End of User Hardware Settings ==================================
#ifdef GPS_PRESENT
#include "gps.h"  // wbp
#endif
#ifdef IR_PRESENT
#include "IRremote.h" // Comment out if IR receiver not present
#include "myIR_Remote.h" // IR Codes defintion file (comment out if IR receiver not present)
#endif
#ifdef RFM12B_PRESENT
#include <RFM12B.h>
#endif

// Pins Declarations
// ===================================================================================
#define BUTTON_DEBOUNCE_TIME 10  // debounce time in ms for the buttons
#define BUTTON_HOLD_TIME 1000  // button hold time before repeating
#define BUTTON_REPEAT_TIME 200  // button repeat interval - 5/second
//#define BOUNCE_TIME_QUICK   50  // bounce time in ms for quickMenu
//#define RESET_BUTTON_TIME 500  // hold time for alarm reset
#define tempPin A0 //Pin for temperature sensor DS18B20
#define photoCellPin A1 // Pin for Photo Resistor
#define SS_SD 4 // Pin for SS on SD Card
//#define SS_RF 18 // Pin for SS on RF12B receiver

const byte MENU_BUTTON_PIN = 28; // "Menu Button" button on analog 4;
const byte SET_BUTTON_PIN = 27;  // "set time" button on digital 27 (analog 3);
const byte INC_BUTTON_PIN = 26;  // "inc time" button on digital 26 (analog 2);

// ===================================================================================

#define heldTime 1000 // Time after which button is considered held

// Days of week bitmask in decimal
// ===================================================================================
#define WEEKDAY 252 // Weekday (Mon-Fri bits are set)
#define DAILY 255 // Daily (Mon-Sun)
#define MON 64 
#define TUE 32 
#define WED 16 
#define THU 8 
#define FRI 4 
#define SAT 2 
#define SUN 1 
// ===================================================================================
// RFM12 Radio module declarations
// ===================================================================================
#define NODEID           1  //network ID used for this unit
#define NETWORKID       1  //the network ID we are on
// Need an instance of the Radio Module
#ifdef RFM12B_PRESENT
RFM12B radio;
#endif
// ===================================================================================
// MISC declarations
// ===================================================================================
#define MAX_MENUS 5 // Maximum number of menus
#define MAX_SUBMENUS 10 // Maximum number of submenus
#define MAX_SETTINGS 8 // Maximum number of time settings menu items
#define MAX_SYSSETTINGS 12 // Maximum number of System menu items
#define MAX_OPTIONS 5 // Max number of options menu items 

#define MAX_VOLUME 7  // loudest volume (0 is lowest)
#define ALARM_STARTVOL 1 // Progressive alarm starting volume
#define ALARM_TONES 12  // number of alarm tones
//#define ALARM_INTERVAL 20 // number of seconds between volume increases (progressive alarm)
#define ALARM_PENDING 60 // number of minutes before a pending alarm that it can be skipped

//#ifdef HAVE_GPS
///volatile uint8_t g_gps_enabled = 2;  // zero: off, 1: 4800 bps, 2: 9600 bps
//volatile int8_t g_TZ_hour = -8;
//volatile int8_t g_TZ_minute = 0;
volatile bool g_gps_updating = false;  // for signalling GPS update on some displays
volatile unsigned long g_gps_timer = 0;  // for tracking how long since GPS last updated
volatile bool g_DST_updated = false;  // DST update flag = allow update only once per day
uint8_t g_DST_Rules[9] = {3,1,2,2,11,1,1,2,1};   // initial values from US DST rules as of 2011
// DST Rules: Start(month, dotw, n, hour), End(month, dotw, n, hour), Offset
// DOTW is Day of the Week, 1=Sunday, 7=Saturday
// N is which occurrence of DOTW
// Current US Rules:  March, Sunday, 2nd, 2am, November, Sunday, 1st, 2 am, 1 hour
//#endif
unsigned long g_IR_timer = 0;  // for tracking how long IR signal LED is on
volatile byte g_GPS_receive = 0;  // flag set to show GPS signal received
volatile byte g_IR_receive = 0;  // flag set to show IR signal received

boolean isSettingTime = false;
boolean isInMenu = false;
boolean isInQMenu = false; // Are we in Quick menu now?
boolean isSettingDate = false;
boolean isSettingAlarm = false;
boolean isSettingSys = false;
boolean isSettingHours   = false;
boolean isSettingMinutes = false;
boolean isSettingMonth   = false;
boolean isSettingDay   = false;
boolean isSettingYear   = false;
boolean isSettingDST = false;
boolean isSettingTZ = false;
boolean isSettingTZMinute = false;
boolean isSettingAlrmMM   = false;
boolean isSettingAlrmHH   = false;
boolean isSettingOptions = false;
boolean okClock = true; // Can we show time? Normally true, unless we showing something else

// button states:
#define BS_OFF       0  // 0 - not down
#define BS_DEBOUNCE  1  // 1 - down, debouncing
#define BS_HOLDING   2  // 2 - held, waiting for repeat
#define BS_RELEASED  3  // 3 - pressed & released, no hold
#define BS_PRESSED   4  // 4 - pressed, still down, not repeating
#define BS_REPEATING 5  // 5 - held, repeat

// Alarm status variables
#define AS_OFF       0
#define AS_WAIT      1  // wait before checking alarm again
#define AS_SKIPPING  2  // skipping pending alarm
#define AS_SOUNDING  3  // sounding now
#define AS_SNOOZING  4  // snoozing now
byte alarmState[2] = {AS_OFF,AS_OFF};  // alarm states
unsigned long alarmTime[2] = {0,0};  // time in ms until next alarm event
unsigned long alarmProgTime[2] = {0,0};  // time in seconds of last alarm volume change

// alarmon moved to Settings.alarmOn
//byte alarmon[2]; // Alarm Freq. Controlled by 8 bits. If first bit is 0 alarm is off. Example in in decimal (not counting 1st bit):
                 // Mon=64, Tue=32, Wed=16, Thu=8, Fri=4, Sat=2, Sun=1, Daily=127, Weekdays=124, Custom=126
byte alarmVol[2]={7,7}; // Alarm Volume (0-12, smaller = louder)

const byte weekdays[8]={0,1,64,32,16,8,4,2}; // Lookup table to convert Weekday number to my day code used for Custom Alarm schedule

boolean isAdjusting = false;  // if on prevents blinking of digits during set
boolean blinking=false;
//boolean buttonPressedInc=false; // Tracks High state of INC button
boolean decrement=false; // Set true to decrement settings instead of increment

unsigned long blinkTime=0; // controls blinking of the dots
unsigned long blinkDigitTime=0; // controls blinking of the digits
boolean blinkOn=true; // blinking objects on (true) or off (false)?
boolean blinkDigit=true; // blinking objects on (true) or off (false)?
unsigned long last_ms=0; // for setting seconds, etc.
unsigned long last_RF=millis(); // Keeps track since last RF signal received
volatile unsigned long lastButtonTime = 0;// last time a button was pushed; used for debouncing

byte clockColor=GREEN; // working (non-permanent) copy of clock Color
byte blinkColor=BLACK; // Default off
byte alarmColor=BLACK; // Default off
byte hhColor=BLACK; // Set color of the 2 hour digits
byte mmColor=BLACK; // Set color of the 2 minute digits
byte monColor=BLACK; // Set color of the month
byte ddColor=BLACK; // Set color of the day
byte yyColor=BLACK; // Set color of the year (last 2 digits)
byte alrmonColor=BLACK; // Set color of the 2 hour digits
byte alrmhhColor=BLACK; // Set color of the 2 hour digits
byte alrmmmColor=BLACK; // Set color of the 2 minute digits
int  extTemp=300; // External Temperature in C
int  extHum=300; // External Humidity

unsigned int photoCell = 0; // last reading from photocell

byte currStatusInc=LOW; // Current Status of Incremental button
byte lastStatusInc=LOW; // Last Status of Incremental button
boolean buttonReleased=false; 

//Temperature chip i/o
#if defined DS18B20
OneWire ds(tempPin); // Connect Temperature Sensor
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&ds);
// arrays to hold device address
DeviceAddress insideThermometer;
#elif defined DHT22
DHT dht(tempPin,DHT22);  // define the temperature/humidity sensor
#endif

byte hours;
byte myhours; // used for 12/24 conversion
byte minutes;
byte seconds;
//int pSeconds; // Precise seconds
byte months;
byte days;
byte years; //Last 2 digits of a year
byte menuItem=0; // Counts presses of the Set button 
byte talkingItem=1; // Talking Menu item number
byte subMenu[MAX_SUBMENUS]={0,0,0,0,0,0,0,0,0,0}; // 0 = setting Alarm1, 1 = setting Alarm 2, 2 for setting Time/Date, 3 for System Settings,
//   4 for setting custom alrm 1, 5 = custom alarm 2, 6 = UserOptions, 7= Infodisplay options, 8 = Voice Prompts, 9 = Photocell
byte lightLevel; // Light level from photocensor
byte prevBrightness = 0; // Previous Brightness (to detect brightness level change)

// Wave Shield Declarations
SdReader card;    // This object holds the information for the card
FatVolume vol;    // This holds the information for the partition on the card
FatReader root;   // This holds the information for the filesystem on the card
FatReader f;      // This holds the information for the file we're play

WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time

/*
void myDebug(){
  //Serial.print ("soundAlarm=");Serial.println(soundAlarm);
   //Serial.print ("interruptAlarm=");Serial.println (interruptAlrm); 
  // Serial.print ("SnozeTime="); Serial.println (snoozeTime); 
}

// Better (low memory) version of Serial print (by David Pankhurst)
// * http://www.utopiamechanicus.com/article/low-memory-serial-print/
void StreamPrint_progmem(Print &out,PGM_P format,...)
{
  // program memory version of printf - copy of format string and result share a buffer
  // so as to avoid too much memory use
  char formatString[128], *ptr;
  strncpy_P( formatString, format, sizeof(formatString) ); // copy in from program mem
  // null terminate - leave last char since we might need it in worst case for result's \0
  formatString[ sizeof(formatString)-2 ]='\0';
  ptr=&formatString[ strlinfoFreqen(formatString)+1 ]; // our result buffer...
  va_list args;
  va_start (args,format);
  vsnprintf(ptr, sizeof(formatString)-1-strlen(formatString), formatString, args );
  va_end (args);
  formatString[ sizeof(formatString)-1 ]='\0';
  out.print(ptr);
}

#define Serialprint(format, ...) StreamPrint_progmem(Serial,PSTR(format),##__VA_ARGS__)
#define Streamprint(stream,format, ...) StreamPrint_progmem(stream,PSTR(format),##__VA_ARGS__)

*/


#if defined DS18B20
// ===================================================================
// * DS18B20 Temperature sensor iniitialization *
// ===================================================================
void TempInit() {
 // ===================================================================
  // Temperature sensor init
  //Serial.print("Locating DS18B20 temperature devices...");
  sensors.begin();
  // find the thermometer
  byte addr = sensors.getAddress(insideThermometer, 0);
//#ifdef PRT_DEBUG
//  putstring("Found ");
//  Serial.print(sensors.getDeviceCount(), DEC);
//  putstring_nl(" devices.");
//  // report parasite power requirements
//  putstring("DS18B20 parasite power is: "); 
//  if (sensors.isParasitePowerMode())  putstring_nl("ON");
//  else  putstring_nl("OFF");
//  if (addr==0)  putstring_nl("Unable to find address for Device 0");
//#endif
  // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensors.setResolution(insideThermometer, 9); 
}
#elif defined DHT22
void TempInit() {
  dht.begin();
}
#endif

// ===================================================================
// * Decode IR Codes *
// Comment out if IR receiver not present
// ===================================================================
#ifdef IR_PRESENT
void IR_process () {
  if (!Settings.IRenabled) return;
  if ((millis()-g_IR_timer) > 1000) { // leave IR signal indicator on for 1 second
    g_IR_timer = millis();  // reset timer
    g_IR_receive = 0;  // no signal
    plot(31,0,BLACK); // clear IR signal indicator
  }
  if (irrecv.decode(&results)) {
//    Serial.print("IR "); Serial.println(results.value, HEX);
    plot(31,0,GREEN); //Plot Indicator dot (IR signal was received)
    g_IR_receive = 1;  // IR signal received
    g_IR_timer = millis();  // reset timer
    switch (results.value) {
      case IR_ON:
//        Serial.println ("Received ON/OFF");
        lastButtonTime=millis();
        processMenuButton(BS_PRESSED);
        break;
      case IR_PLUS:
//        Serial.println ("Received PLUS");
        decrement=false;
        lastButtonTime=millis();
        processIncButton(BS_PRESSED);
        break;
      case IR_MINUS:
//        Serial.println ("Received MINUS");
        decrement=true;
        lastButtonTime=millis();
        processIncButton(BS_PRESSED);
        break;
      case IR_UP:
//        Serial.println ("Received UP");
        lastButtonTime=millis();
        processSetButton(BS_PRESSED);
        break;
      case IR_DOWN:
//        Serial.println ("Received DOWN");
        lastButtonTime=millis();
        processSetButton(BS_PRESSED);
        break;
      case IR_ENTER: // Talk All Items
//        Serial.println ("Received ENTER");
        lastButtonTime=millis();
        quickDisplay(true);
        break;
      case IR_TALK: // Start Talk function
//        Serial.println ("Received MUTE");
        lastButtonTime=millis();
        quickDisplay(false);
        break;
    //Serial.println(results.value, HEX);
    }
    irrecv.resume(); // Receive the next value
  }
}
#endif

//// Timer1 16 bits
//// 16mHz / 16000 = 1000 hZ
//// 9600 BPS = 960 bytes/second - need to get next char at least once/millisecond
//void initTimer1()
//{
//// Inititalize timer1 interrupt for GPS read (& other things?)
//// set timer1 interrupt at 1 kHz
//  TCCR1A = 0; // set entire TCCR1A register to 0
//  TCCR1B = 0; // same for TCCR1B
//  TCNT1  = 0; //initialize counter value to 0
//  // set compare match register for 1 khz interrupts
//  OCR1A = 15999;// = (16*10^6) / (1*1000) - 1 (must be <65536)
//  // turn on CTC mode
//  TCCR1B |= (1 << WGM12);
//  // Set CS10 bit for 1 prescaler
//  TCCR1B |= (1 << CS10);  
//  // enable timer compare interrupt
//  TIMSK1 |= (1 << OCIE1A);  
//}
//
//// 1 tick = ~4 us 
//ISR(TIMER1_OVF_vect)
//{
//  if (isGPSPresent) {
//    GPSread();  // check for data on the serial port
//  }
//}

// Timer2 8 bits
// 16mHz / 8 = 2 mHz = 0.5 uS per tick w/ prescaler
// 2mHz / 256 = 7812.5 Hz overflow frequency
// Timer2 overflow every 0.000128 seconds 
void initTimer2()
{
  // Inititalize timer2 interrupt for GPS read (& other things?)
  TCCR2A &= ~((1<<WGM21) | (1<<WGM20));
  TCCR2B &= ~(1<<WGM22);
  TIMSK2 &= ~(1<<OCIE2A);
  TCCR2B = (1<<CS21); // Set Prescaler to clk/8 
  TIMSK2 |= (1<<TOIE2); // Enable Overflow Interrupt Enable
  TCNT2 = 0; // Initialize counter
}

#ifdef GPS_PRESENT
static uint16_t gps_counter = 0;
//#endif
// runs every 0.000128 seconds
ISR(TIMER2_OVF_vect)
{
  if (Settings.GPSenabled) {
    if (++gps_counter == 4) {  // about once every 0.5 ms
      GPSread();  // check for data on the serial port
      gps_counter = 0;
    }
  }
}

void checkGPS() {
//#ifdef HAVE_GPS
  if (isInMenu) return;
//  if (!g_gps_enabled) return;
  if (!Settings.GPSenabled) return;
  if (gpsDataReady()) {
    parseGPSdata(gpsNMEA());  // get the GPS serial stream and possibly update the clock 
  }
  if ((millis()-g_gps_timer) > 15000) { // how long since GPS signal received?
    g_gps_timer = millis();  // reset timer
    plot(31,1,RED); // set signal received LED to show no signal
    g_GPS_receive = 3; // 
  }
}
#endif

void checkDST() {
//#ifdef HAVE_AUTO_DST
time_t t;
tmElements_t tm;
  if (isInMenu) return;
  if (Settings.DSTmode < 2) return; // nothing to do if DST not set to AUTO
  t = now();
  if ((hour(t) == 0) && (minute(t) == 0) && (second(t) == 0)) {  // MIDNIGHT!
    g_DST_updated = false;
    breakTime(t, tm);
    tm.Year = tmYearToY2k(tm.Year);  // convert (yyyy-1970) year to yy (subtract 30) 
    DSTinit(&tm, g_DST_Rules);  // re-compute DST start, end
  }
  if (second(t) % 10 == 0) { // check DST Offset every 10 seconds (60?)
    setDSToffset(Settings.DSTmode);
  }
//#endif // HAVE_AUTO_DST
}

void setup ()  
{
  Serial.begin(9600);
  
  // ========= Read Settings from EEPROM ===============================
  loadSettings(); // load variables saved in EE (load defaults if needed)
  Settings.clockVer = FIRMWARE_VER;  // update clock firmware version
  clockColor = Settings.clockColor;  // set working copy of clock color

#ifdef RFM12B_PRESENT
  if (!Settings.RadioEnabled) { // Disable saying ext temperature/humidity
    Settings.sayOptions = Settings.sayOptions & (255-(SO_OutTemp+SO_OutHum));
  }
#else
  Settings.RadioEnabled=false;
  Settings.sayOptions = Settings.sayOptions & (255-(SO_OutTemp+SO_OutHum));
#endif
#ifdef DS18B20
  Settings.sayOptions = Settings.sayOptions & (255-SO_InHum);  // DS18B20 doesn't have humidity
#endif
#ifndef IR_PRESENT
  Settings.IRenabled=false;
#endif
#ifndef GPS_PRESENT
  Settings.GPSenabled = false;
#endif

  // Print FW Version
#ifdef PRT_DEBUG
  char welcome[15];
  unsigned int ver = Settings.clockVer;  // 3 digit version number
  byte temp = ver%100; // last 2 digits
  byte ver3 = temp % 10; // Last digit
  byte ver2 = (temp - ver3) / 10; // Second Digit
  ver = (ver - temp) / 100; // First digit
  snprintf(welcome, sizeof(welcome),"Firmware:V%d.%d%d",ver,ver2,ver3); 
  Serial.println (welcome);
#endif
  
  setSyncProvider(RTC.get);   // the function to get the time from the RTC
#ifdef PRT_ERROR
  if(timeStatus()!= timeSet) 
     putstring_nl("Unable to sync with the RTC");
  else
     putstring_nl("RTC has set the system time");
#endif
  ht1632_setup();  // Setup LED Deisplay

  // Uncomment following two lines and modify to set time. After setting time, commend them and re-upload sketch
  //setTime(13,04,0,9,11,12); // Set time on Arduino (hr,min,sec,day,month,yr). Use with RTC.set(now());
  //RTC.set(now()); // Write Time data to RTC Chip. Use with previous command

  WaveShieldInit();
  TempInit();

  // Set initial brightness
  if (Settings.brightness==0) autoBrightness();
  else
    setBrightness(Settings.brightness);

  delay(1000);
//  Serial.print("Set vol "); Serial.println(Settings.soundVol);
  setVol(Settings.soundVol); // Set System Sound Volume
  startup(); // Show welcoming screen

#ifdef GPS_PRESENT
  if (Settings.GPSenabled) {
//    Serial.println("GPS Enabled");
    gpsInit(9600);  // init GPS & Serial port for 9600 BPS
  }
//  else
//    Serial.println("GPS Disabled");
#endif

  // Initialize Radio module 
#ifdef RFM12B_PRESENT
  if (Settings.RadioEnabled) {
//    Serial.println("Radio Enabled");
    radio.Initialize(NODEID, RF12_915MHZ, NETWORKID);
  }
//  else
//    Serial.println("Radio Disabled");
#endif

tmElements_t tm;
  breakTime(now(), tm);
  tm.Year = tmYearToY2k(tm.Year);  // convert (yyyy-1970) year to yy (subtract 30) 
  DSTinit(&tm, g_DST_Rules);  // compute DST start, end	

//  Serial.println (FreeRam());
//  radio.Sleep();

#ifdef IR_PRESENT
  if (Settings.IRenabled) {
    delay(1000); 
//    Serial.println("IR Enabled");
    irrecv.enableIRIn(); // Start the IR receiver. Comment out if IR not present
  }
//  else
//    Serial.println("IR Disabled");
#endif

#ifdef GPS_PRESENT
  initTimer2();  // start timer interrupt running for GPS read (wbp)
#endif

// Test breakTime/makeTime Year handling
//unsigned long tNow = now();
//  Serial.print("tNow "); Serial.println(tNow);
//  Serial.print("year "); Serial.println(year(tNow));
////  tmElements_t tm;
//  breakTime(now(), tm);  // get the time and convert to structure tmElements_t
//  tm.Year = tmYearToY2k(tm.Year);  // convert (yyyy-1970) year to yy (subtract 30) 
//  Serial.print("yr "); Serial.println(tm.Year);
//  tm.Year = y2kYearToTm(tm.Year);  // convert yy year to (yyyy-1970) (add 30) // not for Xronos/Arduino clock
//  tNow = makeTime(tm);  // convert to time_t - seconds since 0/0/1970
//  Serial.print("tNow "); Serial.println(tNow);
//  Serial.print("year "); Serial.println(year(tNow));

}


// =====================================================================================
// >>>>>>>>>>>>>>>>>>>>>>>>> MAIN LOOP <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// =====================================================================================
//unsigned long ctr1 = 0;
//unsigned long time1 = 0;
//unsigned long time2;
// each loop takes on average 13.2 ms - loop runs about 75 times a second
void loop ()
{
//  if (time1==0)
//    time1 = millis();

  setBlinkers();  // set global blink switches
  showBigTime(clockColor);
  procAlarm(0);
  procAlarm(1);
  buttonProc();
//  quickMenu();
//  rearmAlrm(0);
//  rearmAlrm(1);
  infoDisplay();
  autoBrightness();
#ifdef RFM12B_PRESENT
  receiveTemp();
#endif
#ifdef IR_PRESENT
  IR_process();
#endif
#ifdef GPS_PRESENT
  checkGPS();  // wbp
#endif
  checkDST();  // wbp
  saveSettings(false);  // wbp

//  ctr1++;
//  if (ctr1==100) {
//    time2 = millis();
//    Serial.print("time: "); Serial.println((double)(time2-time1)/ctr1,1);
//    time1 = 0;
//    ctr1 = 0;
//  }
	
 //  Serial.println (brightness);

}
// =====================================================================================
// >>>>>>>>>>>>>>>>>>>>>>>>> END MAIN LOOP <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// =====================================================================================

