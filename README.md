# Xronos_Clock
Custom firmware for the Xronos Clock from LensDigital

Latest update Feb 27, 2015 - William Phelps - wm (at) usa.net 
* Ver 2.28 (02/27/2015)
 
## **Includes the following modifications:** ##

* logarithmic brightness levels
* fixed bug: brightness was getting reset to auto
* auto bright - average last 3 readings to smooth out changes
* auto bright - adjust at 0.5 second intervals
* LED indicates PM instead of AM
* Display "Temp In:" instead of "In Temp"
* Display only "Temp:" (no In/Out) if no radio
* GPS support using Serial port
* audio: if 12hr mode & minute == 0, don't say "hundred"

* fix for bug in auto brightness that was crashing clock
* Changed DST setting to "Off, On, Auto"
* Changed date setting order: Month, Day, Year
* Automatic DST (US rules)

* add GPS On/Off in System settings
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
* flip alarm LED colors - green=alarm in next 24 hours, red if not
* add display & set for photocell levels (min, max)
*  BrtLo (0 to 100 by 10) and BrtHi (200 to 750 by 50)
* use structure for settings saved in EE
* fix temp<20 bug in sayTemp(); (from Len)
* fix green/red color bug
* fix debounce timer, hold button to reset alarm
* add button debounce/hold/repeat 
* add tick sound for setting time
* option to enable/disable blinking colon
* global blink timers, bug fixes
* rewrite button logic, merge button checks to common routine
* add "alarm reset" scroll message
* hold Alarm Reset to skip upcoming alarm
