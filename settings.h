/***********************************************************************
 * February 2015 - mods by WBPHELPS
 * 
 * (c) 2015 W. B. Phelps / Meier-Phelps Consulting
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// infoOptions values
const byte IO_Date    = B10000000;  // Scroll Date
const byte IO_Alarms  = B01000000;  // Scroll Alarms
const byte IO_InTemp  = B00100000;  // Scroll Inside Temp
const byte IO_InHum   = B00010000;  // Scroll Inside Humidity
const byte IO_Sensor  = B00001000;  // Scroll Sensor status
const byte IO_OutTemp = B00000100;  // Scroll Outside Temp
const byte IO_OutHum  = B00000010;  // Scroll Outside Humidity
//const byte IO_???    = B00000001;  // 

// sayOptions values
const byte SO_Time    = B10000000;  // Speak Time
const byte SO_Date    = B01000000;  // Speak Date
const byte SO_InTemp  = B00100000;  // Speak Inside Temp
const byte SO_InHum   = B00010000;  // Speak Inside Humidity
const byte SO_Alarms  = B00001000;  // Speak Alarms
const byte SO_OutTemp = B00000100;  // Speak External Temp (Radio)
const byte SO_OutHum  = B00000010;  // Speak External Humidity (Radio)

struct __Settings
{
  byte EEcheck1; // EE version check

  unsigned int clockVer; // Firmware version
  boolean time12hr; // 12/24 hour mode
  byte brightness; // brightness
  byte clockColor; // time digit color
  boolean autoColor; // Auto Color enabled
  byte clockFont; // Clock's font number
  boolean cursorBlink; // blinking colon control
  byte sFX; // Menu SFX on/off
  boolean tempUnit; // Degree units C/F
  byte soundVol; // Sound Volume
  byte infoFreq; // Info Display frequency and what to show
  byte infoOptions; // Info Display frequency and what to show
  byte sayOptions; // Define which options to speak
  byte startup; // Display startup message?
  byte tempOffset; // Temperature Offset
  byte RadioEnabled; // Defines if RF receiver is enabled
  byte IRenabled; // Defines if IR receiver is enabled
  byte DSTmode; // DST mode
  byte DSToffset; // current DST offset
  byte GPSenabled; // GPS receiver enabled
  byte alarmHH[2]; // Alarm hours
  byte alarmMM[2]; // Alarm minutes
  // Alarm Freq. Controlled by 8 bits. If first bit is 0 alarm is off. Example in in decimal (not counting 1st bit):
  // Mon=64, Tue=32, Wed=16, Thu=8, Fri=4, Sat=2, Sun=1, Daily=127, Weekdays=124, Custom=126
  byte alarmOn[2]; // Alarm Off/Daily/Weekday/Custom settings
  boolean alarmCustom[2]; // custom alarm schedule
  byte alarmTone[2]; // Alarm tone
  byte alarmProgVol[2]; // Alarm progressive volume?
  unsigned int photoCellMin; // Photocell minimum value
  unsigned int photoCellMax; // Photocell maximum value
  byte alarmSnoozeTime;  // how long to snooze for, in minutes

  byte EEcheck2; // EE version check
};

void loadSettings(void);
void saveSettings(void);
void timeSettings(void);
extern struct __Settings Settings;

#endif

