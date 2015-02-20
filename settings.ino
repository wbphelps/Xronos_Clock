/*

 */

/*

 ========================================
 EEPROM write/read functions
 ========================================
 
 */

#include "settings.h"

#include <avr/eeprom.h>
#include <EEPROM.h>

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define EE_Address 32 // eeprom starting address

__Settings Settings = {
  EE_VERSION, // EE version;

  FIRMWARE_VER, // firmware version - why is this in EE?
  true, // 12 hour
  0, // brightness (auto)
  GREEN, // clock color
  true, // auto color
  1, // clock font
  true, // blinking colon
  true, // SFX
  true, // temperature units
  0, // sound volume
  2, // info freq
  2, // info options
  255, // say options
  true, // do startup
  2, // temperature offset
  false, // radio receiver
  false, // IR receiver
  false, // DST mode
  false, // GPS on
  {0,0}, // alarm hours
  {0,0}, // alarm minutes
  {127,127}, // alarm on/off etc - default: off, daily 
  {0,0}, // alarm custom schedule
  {1,1}, // alarm tone
  {false,false}, // alarm progressive volume
  10, // photocell minimum
  400, // photocell maximum

  EE_VERSION, // EE version;
};

static unsigned long Settings_timer = 0;
// reset settings timer 
void timeSettings(void){
  Settings_timer = millis();
}

// Save settings array in EE
void saveSettings(uint8_t quiet) {
  uint8_t c1 = 0; // # of bytes written
  if ((millis()-Settings_timer) < 5000)  return; // check & update EE every 5 seconds
  Settings_timer = millis();
  for (unsigned int p=0; p<sizeof(Settings); p++) {
    uint8_t b1 = eeprom_read_byte((uint8_t *)EE_Address + p);
    uint8_t b2 = *((uint8_t *) &Settings + p);
    if (b1 != b2) {
      eeprom_write_byte((uint8_t *)EE_Address + p, *((uint8_t*)&Settings + p));
      c1++;
    }
  }
  if (c1) {  // did anything change?
    if (quiet==0)
    //tone(PinMap::piezo, 1760, 25);  // short beep (almost a tick)
    playSFX(4);  // play a little tone to show EE memory saved
  }
}

// Load Settings array from EE; initialize array if EE never set
void loadSettings(void) {
  uint8_t ee_check1 = eeprom_read_byte((uint8_t *)EE_Address + (&Settings.EEcheck1-&Settings.EEcheck1));
  uint8_t ee_check2 = eeprom_read_byte((uint8_t *)EE_Address + (&Settings.EEcheck2-&Settings.EEcheck1));
  if ((ee_check1!=EE_VERSION) || (ee_check2!=EE_VERSION)) { // has EE been initialized?
    for (unsigned int p=0; p<sizeof(Settings); p++) { // copy Settings structure to EE memory
      eeprom_write_byte((uint8_t *)EE_Address + p, *((uint8_t*)&Settings + p));
    }
  }
  else { // read Settings from EE
    for (unsigned int p=0; p<sizeof(Settings); p++) // read gloabls from EE
      *((uint8_t*)&Settings + p) = eeprom_read_byte((uint8_t *)EE_Address + p);
  }
}

//    // Check if custom alarm schdule is set
//    alrmTst=alarmon[i]<<1; // Shift left one to get rid of 1st bit
//    alrmTst=alrmTst>>1; // Shift right one to get rid of 1st bit
//    if ( (alrmTst == 124) || (alrmTst == 127) ) isSettingAlrmCust[i]=false; // i.e. Daily or Weekday alarm is set
//    else isSettingAlrmCust[i]=true; // It's set to custom day
//  }

//  if ( RFM12B_PRESENT ) Settings.RadioEnabled=EEPROM.read (radioOnLoc);
//  else Settings.RadioEnabled=false;

//  if (!Settings.RadioEnabled) { // Disable saying ext temperature/humidity
//    if (Settings.sayOptions & 4) Settings.sayOptions = Settings.sayOptions ^ 4;
//    if (Settings.sayOptions & 2) Settings.sayOptions = Settings.sayOptions ^ 2;
//  }

//  if ( IR_PRESENT ) Settings.IRenabled=EEPROM.read (IROnLoc); // IR receiver setting
//  else Settings.IRenabled=false;
//  if ( GPS_PRESENT ) Settings.GPSenabled=EEPROM.read (GPSOnLoc); // IR receiver setting
//  else Settings.GPSenabled = false;


