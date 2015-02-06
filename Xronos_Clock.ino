/***********************************************************************
* December 2014, January 2015 - mods by WBPHELPS
* Ver 2.18 (01/14/2015)
* logarithmic brightness levels
* bugfix: brightness set to auto by error
* auto bright - adjust at 1 second intervals (was 10)
* auto bright - average last 3 readings to smooth out changes
* PM LED instead of AM
* "Temp In:" instead of "In Temp"
* "Temp:" (no In/Out) if no radio
* GPS support
* change colon display & offset
* audio: if 12hr mode & minute == 0, don't say "hundred"
* fix bug in brightness averaging
* DST setting: Off, On, Auto
* AUTO DST working!
* Add GPS On/Off in System settings
* switch IRRemote to use Timer3 interrupt instead of Timer2
* small changes to fonts 3 & 4
* set IR & GPS status LED's after clear
* adjust some menu spacings
* auto color, depending on light level
* move alarm indicators to edges
* change menu timeout to 5 seconds
* blink alarm indicators if playing or snoozing
* add progressive alarm volume on/off option for alarm setting
* set alarm LED color if alarm is due within next 24 hours
* atomic (single fetch) time/date
*
* Add TZ Hr & TZ Mn to settings?
* more compact text scrolling
*
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
#include <Wire.h>
#include "HT1632.h"
#include <avr/pgmspace.h>
#include <Time.h>  
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t
#include <stdlib.h> // Used for string manipulations and string to int conversions
#include "gps.h"  // wbp
#include "WaveUtil.h" // Used by wave shield
#include "WaveHC.h" // Used by wave shield (library modified by LensDigital to accomodate ATMega644p/ATMega1284p)
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <RFM12B.h>
#include "IRremote.h" // Comment out if IR receiver not present
#include "myIR_Remote.h" // IR Codes defintion file (comment out if IR receiver not present)

//#define firmware_ver 209 // Current Firmware version
#define firmware_ver 218 // Current Firmware version (wbp)

// ============================================================================================
// Importante User Hardware config settings, modify as needed
// ============================================================================================
static boolean RFM12B_Enabled=true; // Defines if RFM12B Chip present.  Set to true to enable. Must also have ATMega1284p! Will not work with ATMega644p chip
#define AUTO_BRIGHTNESS_ON 0  //Set to 1 to disable autobrightness menu feature, 0 to enable if photocell is present.
static boolean IR_PRESENT=true; // Set to True if IR receiver is present. Must also have ATMega1284p! Will not work with ATMega644p chip
static boolean GPS_PRESENT=true; // Set to True if GPS receiver is present
// ============================End of User Hardware Settings ==================================

// Pins Delcarations
// ===================================================================================
#define BOUNCE_TIME_BUTTON  200   // bounce time in ms for the menu button
#define BOUNCE_TIME_QUICK   50  // bounce time in ms for quickMenu
#define tempPin A0 //Pin for temperature sensor DS18B20
#define photoCellPin A1 // Pin for Photo Resistor
#define SS_SD 4 // Pin for SS on SD Card
//#define SS_RF 18 // Pin for SS on RF12B receiver
#define MENU_BUTTON_PIN A4// "Menu Button" button on analog 4;
#define SET_BUTTON_PIN A3// "set time" button on digital 17 (analog 3);
#define INC_BUTTON_PIN A2// "inc time" button on digital 16 (analog 2);
#define PHOTOCELL_MIN 20 // Minimum reading from Photocell (wbp)
#define PHOTOCELL_MAX 800 // Maximum reading from Photocell (for brightess LED level)
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
RFM12B radio;
// ===================================================================================
// MISC declarations
// ===================================================================================
#define MAX_MENUS 5 // Maximum number of menus
#define MAX_SUBMENUS 8 // Maximum number of menus
#define MAX_SETTINGS 6 // Maximum number of settings menu items
#define MAX_SYSSETTINGS 11 // Maximum number of System menu items

//#define ALARM_PROGRESSIVE 6 // Number of alarm sounds that have progressive volume

//#ifdef HAVE_GPS
///volatile uint8_t g_gps_enabled = 2;  // zero: off, 1: 4800 bps, 2: 9600 bps
volatile int8_t g_TZ_hour = -8;
volatile int8_t g_TZ_minute = 0;
//volatile bool g_gps_signal = false;  // GPRMC message received
volatile bool g_gps_updating = false;  // for signalling GPS update on some displays
volatile unsigned long g_gps_timer = 0;  // for tracking how long since GPS last updated
volatile uint8_t g_DST_mode = 0;  // off: 0, on: 1, auto: 2
volatile int8_t g_DST_offset = 0;
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
boolean isSettingAlrmMM   = false;
boolean isSettingAlrmHH   = false;
boolean isSettingAlrmCust[2]; // Use to track custom alarm schedule setting
boolean isAlarmModified[2]={false,false}; // If hh:mm of alarm was changed, settings will be written to EEPROM
boolean isSettingOptions = false;
boolean okClock = true; // Can we show time? Normally true, unless we showing something else
boolean time12hr; // Showing time in 24 or 12 hr format
boolean interruptAlrm[2] = {false,false};
boolean soundAlarm[2] = {false,false};
boolean interruptAlrm2 = false;
boolean soundAlarm2 = false;
boolean isIncrementing = false;
boolean blinking=false;
boolean sFX=true; // Menu Effects on/off
boolean buttonPressedInc=false; // Tracks High state of INC button
boolean doStartup; // Startup sequence enable/disable
boolean radioOn=true;
boolean isRadioPresent; // Defines in RFM12B Chip present. Stored in EEPROM
boolean isIRPresent; // Defines if IR receiver is present
boolean isGPSPresent; // Defines if GPS receiver is present
boolean decrement; // Only used with IR remote to decrement digits (--)


byte alrmHH[2]; // Alarm Hours
byte alrmMM[2]; // Alarm Minutes
byte alarmon[2]; // Alarm Freq. Controlled by 8 bits. If first bit is 0 alarm is off. Example in in decimal (not counting 1st bit):
                 // Mon=64, Tue=32, Wed=16, Thu=8, Fri=4, Sat=2, Sun=1, Daily=127, Weekdays=124, Custom=126
byte alrmVol[2]={7,7}; // Alarm Volume (0-12, smaller = louder)
byte alrmProgVol[2]={1,1}; //Progressive volume?
byte alrmToneNum[2]; // Number of alarm tone

byte tmpOffset; // temperature offset (minus)
byte sndVol=0; // Normal Sounds volume (0-12, smaller = louder)

const byte weekdays[8]={0,1,64,32,16,8,4,2}; // Lookup table to convert Weekday number to my day code used for Custom Alarm schedule

unsigned long blinkTime=0; // controls blinking of the dots
unsigned long alarmBlinkTime=0; // controls blinking of alarm indicators
unsigned long last_ms=0; // for setting seconds, etc.
unsigned long last_RF=millis(); // Keeps track since last RF signal received
volatile unsigned long lastButtonTime = 0;// last time a button was pushed; used for debouncing

byte clockColor;
byte autoColor = true;  // Auto color mode
byte clockFont; 
byte blinkColor=BLACK; // Default off
byte alarmColor=BLACK; // Default off
byte hhColor=BLACK; // Set color of the 2 hour digits
byte mmColor=BLACK; // Set color of the 2 minute digits
byte monColor=BLACK; // Set color of the month
byte ddColor=BLACK; // Set color of the day
byte yyColor=BLACK; // Set color of the year (last 2 digits)
byte dateColor=BLACK; // Set color of the date
byte alrmonColor=BLACK; // Set color of the 2 hour digits
byte alrmhhColor=BLACK; // Set color of the 2 hour digits
byte alrmmmColor=BLACK; // Set color of the 2 minute digits
byte snoozeTime[2]={10,10}; // Keeps last digit of minutes for snooze
int  extTemp=300; // External Temperature in C
int  extHum=300; // External Humidity

byte currStatusInc=LOW; // Current Status of Incremental button
byte lastStatusInc=LOW; // Last Status of Incremental button
boolean buttonReleased=false; 

//Temperature chip i/o
OneWire ds(tempPin); // Connect Temperature Sensor
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&ds);
// arrays to hold device address
DeviceAddress insideThermometer;

byte hours;
byte myhours; // used for 12/24 conversion
byte minutes;
byte seconds;
//int pSeconds; // Precise seconds
byte months;
byte days;
byte years; //Last 2 digits of a year
byte menuItem=0; // Counts presses of the Set button 
byte mbutState=1; // Menu button option 
byte subMenu[MAX_SUBMENUS]={0,0,0,0,0,0,0,0}; // 0 = setting Alarm1, 1 = setting Alarm 2, 2 for setting Time/Date, 3 for System Settings, 4 for setting custom alrm 1, 5 = custom alarm 2, 6 = UserOptions, 7= Infodisplay options, 8 = Voice Prompts
byte brightness; // LED Display Brightness
byte lightLevel; // Light level from photocensor
byte prevBrightness = 0; // Previous Brightness (to detect brightness level change)
boolean tempUnit; // Temperature units (True=F or False=C)
byte infoFreq; // Info Display Freq options
byte sayOptions;  // Say items options
byte infoOptions;  // Info Display items options

// EEPROM Location Definitions
#define	mode24HRLoc 0		// 12/24 hour mode storage location
const byte alarmHHLoc[2]={1,9};                        // alarm hours storage location
const byte alarmMMLoc[2]={2,10};           		// alarm minutes storage location
const byte alarmOnOffLoc[2]={3,11};		        // alarm Off/Daily/Weekday/Custom storage locations
#define	brightLoc 4		// Brightness storage location
#define	clockColorLoc 5		// Time digit color storage location
#define	clockVerLoc 6		// Firmware version storage location
const byte alarmToneLoc[2]={7,12};                    // Alarm  Tone storage location
#define clockFontLoc 8               // Clock's font number location
#define sFXLoc 13               // Menu SFX on/off location
#define tempUnitLoc 14               // Degreen Units location
#define sndVolLoc 15               // Sound Volume control location
#define infoFreqLoc 16               // Info Display frequency and what to show
#define sayOptionsLoc 17               // Define which options to say
#define doStartupLoc 18               
#define tmpOffsetLoc 20              // Temperature Offset sotrage location
#define infoOptionsLoc 21               // Info Display frequency and what to show
#define radioOnLoc 22          // Defines if RF receiver is enabled
#define IROnLoc 23          // Defines if IR receiver is enabled
#define DSTmodeLoc 24   // DST mode in EE
#define GPSOnLoc 25   // GPS receiver enabled in EE
#define autoColorLoc 26  // Auto Color enabled in EE
const byte alarmProgVolLoc[2]={3,11};		        // alarm Off/Daily/Weekday/Custom storage locations

// Wave Shield Declarations
SdReader card;    // This object holds the information for the card
FatVolume vol;    // This holds the information for the partition on the card
FatReader root;   // This holds the information for the filesystem on the card
FatReader f;      // This holds the information for the file we're play

WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time
// we will track if a button is just pressed, just released, or 'pressed' (the current state
//volatile byte pressed[2], justpressed[2], justreleased[2];

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


// ===================================================================
// * DS18B20 Temperature sensor iniitialization *
// ===================================================================
void TempInit(){
 // ===================================================================
  // Temperature sensor init
  //Serial.print("Locating DS18B20 temperature devices...");
  sensors.begin();
  putstring("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  putstring_nl(" devices.");
  // report parasite power requirements
  putstring("DS18B20 parasite power is: "); 
  if (sensors.isParasitePowerMode()) putstring_nl("ON");
  else putstring_nl("OFF");
   if (!sensors.getAddress(insideThermometer, 0)) putstring_nl("Unable to find address for Device 0"); 
 // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensors.setResolution(insideThermometer, 9); 
  
}



// ===================================================================
// * READ EEPROM Settings *
// ===================================================================
void getEEPROMSettings () {
  byte alrmTst;
  for (byte i=0;i<2;i++) {
    alarmon[i]=EEPROM.read(alarmOnOffLoc[i]); // read Alarm off/Daily/Weekday from EEPROM
    alrmHH[i]=EEPROM.read(alarmHHLoc[i]);  // Read Alarm Hours from EEPROM
    alrmMM[i]=EEPROM.read(alarmMMLoc[i]); // Read Alarm Minutes from EEPROM
    alrmToneNum[i]=EEPROM.read(alarmToneLoc[i]); // Read Alarm Tone number from EEPROM
    // Check if custom alarm schdule is set
    alrmTst=alarmon[i]<<1; // Shift left one to get rid of 1st bit
    alrmTst=alrmTst>>1; // Shift right one to get rid of 1st bit
    if ( (alrmTst == 124) || (alrmTst == 127) ) isSettingAlrmCust[i]=false; // i.e. Daily or Weekday alarm is set
    else isSettingAlrmCust[i]=true; // It's set to custom day
  }
  time12hr=EEPROM.read(mode24HRLoc); // Read time mode (12/24 hours) from EEPROM
  brightness=EEPROM.read(brightLoc); // Read Brightness setting from EEPROM
  clockColor=EEPROM.read(clockColorLoc); // Read Clock Color from EEPROM
  if (clockColor!=RED && clockColor!=GREEN && clockColor!=ORANGE) clockColor=ORANGE;// Failsafe for when EEPROM location is blank or corrupted, so clock will be visible
  autoColor=EEPROM.read(autoColorLoc); // Read Auto Color from EEPROM
  clockFont=EEPROM.read(clockFontLoc); // Read Alarm Tone number from EEPROM
  sFX=EEPROM.read(sFXLoc); // Read Sound FX on/of for menu system
  tempUnit=EEPROM.read(tempUnitLoc); // Read temp unit
  sndVol=EEPROM.read(sndVolLoc); // Read Sound Volume
  infoFreq=EEPROM.read(infoFreqLoc); // Read infodisplay options
  infoOptions=EEPROM.read(infoOptionsLoc); // Read infodisplay options
  sayOptions=EEPROM.read(sayOptionsLoc); // Read say prompt options
  doStartup=EEPROM.read(doStartupLoc); // Read say prompt options
  tmpOffset=EEPROM.read (tmpOffsetLoc); // Read Temperature offset
  g_DST_mode=EEPROM.read (DSTmodeLoc); // Read DST mode
  if ( RFM12B_Enabled ) isRadioPresent=EEPROM.read (radioOnLoc);
  else isRadioPresent=false;
  
  if (!isRadioPresent) { // Disable saying ext temperature/humidity
    if (sayOptions & 4) sayOptions = sayOptions ^ 4;
    if (sayOptions & 2) sayOptions = sayOptions ^ 2;
  }
  if (EEPROM.read (clockVerLoc) != firmware_ver) { //Write current firmware version to EEPROM if it's different from stored
    putstring_nl("Writing new Fw version");
    EEPROM.write (clockVerLoc,firmware_ver); 
    delay (15); 
  }
  if ( IR_PRESENT ) isIRPresent=EEPROM.read (IROnLoc); // IR receiver setting
  else isIRPresent=false;
  if ( GPS_PRESENT ) isGPSPresent=EEPROM.read (GPSOnLoc); // IR receiver setting
  else isGPSPresent=false;
}

// ===================================================================
// * Decode IR Codes *
// Comment out if IR receiver not present
// ===================================================================
void IR_process () {
  if (!isIRPresent) return;
  if ((millis()-g_IR_timer) > 1000) { // leave IR signal indicator on for 1 second
    g_IR_timer = millis();  // reset timer
    g_IR_receive = 0;  // no signal
    plot(31,2,BLACK); // clear IR signal indicator
  }
  if (irrecv.decode(&results)) {
//    Serial.println(results.value, HEX);
    plot(31,2,GREEN); //Plot Indicator dot (IR signal was received)
    g_IR_receive = 1;  // IR signal received
    g_IR_timer = millis();  // reset timer
    switch (results.value) {
      case IR_ON:
        //Serial.println ("Received ON/OFF");
        lastButtonTime=millis()+ BOUNCE_TIME_BUTTON;
        processMenuButton();
        break;
      case IR_PLUS:
        //Serial.println ("Received PLUS");
        decrement=false;
        lastButtonTime=millis()+ BOUNCE_TIME_BUTTON;
        processIncButton();
        break;
      case IR_MINUS:
        //Serial.println ("Received MINUS");
        decrement=true;
        lastButtonTime=millis()+ BOUNCE_TIME_BUTTON;
        processIncButton();
        break;
      case IR_UP:
        //Serial.println ("Received UP");
        lastButtonTime=millis()+ BOUNCE_TIME_BUTTON;
        processSetButton();
        break;
      case IR_DOWN:
        //Serial.println ("Received DOWN");
        lastButtonTime=millis()+ BOUNCE_TIME_BUTTON;
        processSetButton();
        break;
      case IR_ENTER: // Talk All Items
        //Serial.println ("Received ENTER");
        lastButtonTime=millis()+BOUNCE_TIME_QUICK;
        buttonReleased=true;
        last_ms=millis()+heldTime;
        quickDisplay();
        break;
      case IR_TALK: // Start Talk function
        //Serial.println ("Received MUTE");
        lastButtonTime=millis()+BOUNCE_TIME_QUICK;
        buttonReleased=true;
        last_ms=millis();
        quickDisplay();
        break;
    //Serial.println(results.value, HEX);
    }
    irrecv.resume(); // Receive the next value
  }
}

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

//#ifdef HAVE_GPS
static uint8_t gps_counter = 0;
//#endif
// runs every 0.000128 seconds
ISR(TIMER2_OVF_vect)
{
  if (isGPSPresent) {
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
  if (!isGPSPresent) return;
  if (gpsDataReady()) {
    parseGPSdata(gpsNMEA());  // get the GPS serial stream and possibly update the clock 
  }
  if ((millis()-g_gps_timer) > 15000) { // how long since GPS signal received?
    g_gps_timer = millis();  // reset timer
    plot(31,1,RED); // set signal received LED to show no signal
    g_GPS_receive = 3; // 
  }
}

void checkDST() {
//#ifdef HAVE_AUTO_DST
time_t t;
tmElements_t tm;
  if (isInMenu) return;
  if (g_DST_mode < 2) return; // nothing to do if DST not set to AUTO
  t = now();
  if ((hour(t) == 0) && (minute(t) == 0) && (second(t) == 0)) {  // MIDNIGHT!
    g_DST_updated = false;
    breakTime(t, tm);
    DSTinit(&tm, g_DST_Rules);  // re-compute DST start, end
  }
  if (second(t) % 10 == 0) { // check DST Offset every 10 seconds (60?)
    setDSToffset(g_DST_mode);
  }
//#endif // HAVE_AUTO_DST
}

void setup ()  
{
  Serial.begin(9600); // (115200); (wbp)
  //initEPROM();
  
  // Print FW Version
    char welcome[15];
    byte ver=EEPROM.read (clockVerLoc); // Read 3 digit version number
    byte temp = (ver%100) %10; //temp holder
    byte ver3 = temp % 10; // Last digit
    byte ver2 = (temp - ver3) / 10; // Second Digit
    ver = (ver - ver2) / 100; // First digitf
    snprintf(welcome, sizeof(welcome),"Firmware:V%d.%d%d",ver,ver2,ver3); 
    Serial.println (welcome);
  setSyncProvider(RTC.get);   // the function to get the time from the RTC
  if(timeStatus()!= timeSet) 
     putstring_nl("Unable to sync with the RTC");
  else
     putstring_nl("RTC has set the system time");    
  ht1632_setup();  // Setup LED Deisplay
  //setBrightness(brightness); // Set Brightness 
  // Uncomment following two lines and modify to set time. After setting time, commend them and re-upload sketch
  //setTime(13,04,0,9,11,12); // Set time on Arduino (hr,min,sec,day,month,yr). Use with RTC.set(now());
  //RTC.set(now()); // Write Time data to RTC Chip. Use with previous command
   //EEPROM.write (clockVerLoc,firmware_ver); delay (50); // Write current firmware version to EEProm
   // ========= Read Settings from EEPROM ===============================
  getEEPROMSettings ();
  wave.volume=sndVol; // Change System Sound Volume
  WaveShieldInit();
  TempInit();
  // Set initial brightness
  if (brightness==0) autoBrightness();
  else
    setBrightness(brightness);
  // Initialize Radio module 
  if (isRadioPresent) radio.Initialize(NODEID, RF12_915MHZ, NETWORKID);
//  if (isIRPresent) {
//    irrecv.enableIRIn(); // Start the IR receiver. Comment out if IR not present
//  }

  gpsInit(9600);  // init GPS & Serial port for 9600 BPS
//	if (g_DST_mode == 2)  // DST set to AUTO?
tmElements_t tm;
  breakTime(now(), tm);
  DSTinit(&tm, g_DST_Rules);  // compute DST start, end	

  startup(); // Show welcoming screen
//  Serial.println (FreeRam());
//  radio.Sleep();

  delay(1000); 
  if (isIRPresent) {
    Serial.println("IR Present");
    irrecv.enableIRIn(); // Start the IR receiver. Comment out if IR not present
  }
  else
    Serial.println("NO IR!");

  initTimer2();  // start timer interrupt running for GPS read (wbp)

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

  showBigTime(clockColor);
  procAlarm(0);
  procAlarm(1);
  buttonProc();
  quickMenu();
  rearmAlrm(0);
  rearmAlrm(1);
  infoDisplay();
  autoBrightness();
  receiveTemp();
  IR_process();
  checkGPS();  // wbp
  checkDST();  // wbp

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

