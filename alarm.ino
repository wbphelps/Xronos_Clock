// =======================================================================================
// ---- Calculate time until next alarm, in seconds ----
// By: wbp
// only checks alarm times for today & possibly tomorrow
// returns 9999 if alarm not happening within 24 hours
// =======================================================================================
int timeToNextAlarm(byte alrmnum) {
  int dAlrm, iAlrm, iNow, wd;
  unsigned long tNow = now();
  wd = weekday(tNow);  // today's weekday number
  dAlrm = 9999; // assume no alarm within 24 hours
  if (Settings.alarmOn[alrmnum] & 128) {  // is alarm on?
    iAlrm = Settings.alarmHH[alrmnum]*60 + Settings.alarmMM[alrmnum];  // alarm time in minutes
    iNow = hour(tNow)*60 + minute(tNow);  // time now in minutes
    if (iNow>iAlrm) { // is iAlrm in the past?
      iAlrm+=1440;  // set iAlrm ahead by one day
      wd+=1;  // let's look at tomorrow
      if (wd>7)  wd=1;  // wrap if necessary
    }
    if (Settings.alarmOn[alrmnum] & weekdays[wd])  // is set for the day in question?
      dAlrm = iAlrm-iNow;  // calculate how long until next alarm
  }
  return dAlrm;
}

// =======================================================================================
// ---- Process Alarm Function ----
// By: LensDigital
// =======================================================================================
//static unsigned long alarmBlinkTime=0; // controls blinking of alarm indicators
void procAlarm(byte alrmnum) {
  int blinkDuration = 1000; // How frequently dots should blink (wbp)
  unsigned long tNow = now();  // time now in seconds
  if (isInMenu) return; // Do not sound alarm if changing settings
///  if (digitalRead(SET_BUTTON_PIN) == HIGH) processSetButton(); // Poll Set Button, that will reArm (cancel for today) alarm  ???
  if (Settings.alarmOn[alrmnum] & 128) { // Is global alarm switch on? (1st bit is set)

// ==== Begin alarm LED indicator ====

      if (alarmState[alrmnum] == AS_SOUNDING) { // Alarm currently sounding?
        if (blinkOn)
          if (alarmState[alrmnum] == AS_SKIPPING)
            alarmColor=ORANGE;  // blink alarm indicator
          else
            alarmColor=RED;  // blink alarm indicator
        else alarmColor=BLACK;
        plot (31*alrmnum,14,alarmColor); // Show blinking dot in Orange if snoozing  (wbp)
      }

      else if ( alarmState[alrmnum] == AS_SNOOZING)  { // snoozing
        if (blinkOn)  alarmColor=ORANGE;  // use Yellow for snoozing alarm
        else alarmColor=BLACK;  // and blink it
        plot (31*alrmnum,14,alarmColor); // Show blinking dot in Orange if snoozing  (wbp)
      }

      else  { // not sounding and not snoozing...
        // check to see if alarm will sound again within 24 hours
        if (timeToNextAlarm(alrmnum) < 1440) {
          if (alarmState[alrmnum] == AS_SKIPPING)
            alarmColor=ORANGE;  // change LED to yellow
          else
            alarmColor=GREEN;  // change LED to green
        }
        else
          alarmColor=RED;  // assume it won't
        plot (alrmnum*31,14,alarmColor); // show alarm status
      } 
//  }
// ==== END alarm LED indicator ====

    if (alarmState[alrmnum] == AS_WAIT) {  // waiting?  (set so alarm doesn't restart if cancelled in 1st minute
      if ( (tNow - alarmTime[alrmnum]) > 60 ) {  // wait for 1 minute to allow time to change
        alarmState[alrmnum] = AS_OFF;  // reset alarm state
//        Serial.print("alarmState: "); Serial.println(alarmState[alrmnum]);
      }
    }
    else if (alarmState[alrmnum] == AS_SOUNDING) {  // sounding the alarm?
      if ( (tNow - alarmTime[alrmnum]) > 60*60 ) {  // has alarm been sounding for 60 minutes?
        alarmState[alrmnum] = AS_OFF;  // stop sounding alarm
        alarmTime[alrmnum] = tNow;  // remember when state was changed
//        Serial.print("alarmState: "); Serial.println(alarmState[alrmnum]);
        if (Settings.alarmProgVol[alrmnum]) alrmVol[alrmnum]=ALARM_PROG_STARTVOL; //Set low volume for escalating alarms (wbp)
        else alrmVol[alrmnum]=0; // Set High volume for non-escalating alarms
      }
      else
        playAlarm(alrmnum);  // restart alarm sound if needed
    }
    // ==== Begin Snooze Check ====
    else if ( alarmState[alrmnum] == AS_SNOOZING ) {  // Are We Snoozing?
      // Is it time to reset Snooze?
      if ( (tNow - alarmTime[alrmnum]) > (Settings.alarmSnoozeTime*60) ) {  // snooze time 9 minutes (for now)
        alarmState[alrmnum] = AS_SOUNDING;  // turn alarm back on
        alarmTime[alrmnum] = tNow;  // remember when state was changed
//        Serial.print("alarmState: "); Serial.println(alarmState[alrmnum]);
        if (Settings.alarmProgVol[alrmnum])  // progressive alarm volume?
          alrmVol[alrmnum]=ALARM_PROG_STARTVOL; // Reset Alarm Volume
        else  alrmVol[alrmnum]=0;  // alarm volume to max
      }
    }
    // ==== End Snooze Check ====
    // Maybe it's time to sound alarm for the first time at specified time?
    else if ( alarmState[alrmnum] < AS_SOUNDING ) { // Alarm is not sounding or snoozing - is it time to do something?
      if ( Settings.alarmOn[alrmnum] & weekdays[weekday()] ) { // Alarm is scheduled for this day!
        if ( (hour(tNow)==Settings.alarmHH[alrmnum]) && ( minute(tNow)==Settings.alarmMM[alrmnum]) ) {  // time to sound alarm?
          if (alrmnum==0) {  // is this first alarm or second?
            alarmState[1] = AS_OFF;  // cancel alarm 2 
          }
          else { // It's second alarm processor
            alarmState[0] = AS_OFF;  // cancel alarm 1
          }
          if (alarmState[alrmnum] == AS_SKIPPING)  // is this alarm being skipped?
            alarmState[alrmnum] = AS_WAIT; // Alarm time reached, turn it off without sounding it
          else
            alarmState[alrmnum] = AS_SOUNDING; // If it's time to sound alarm!
          alarmTime[alrmnum] = tNow;  // remember when state was changed
//          Serial.print("alarmState: "); Serial.println(alarmState[alrmnum]);
        }
      }
    }
  }
}



// =======================================================================================
// ---- Play Alarm Sound ----
// By: LensDigital
// =======================================================================================
void playAlarm(byte alrmnum) {
//  Serial.print("playAlarm: "); Serial.print(alrmnum); Serial.print(", ");
  turnOffRadio(); // Disable RF12B
//  if (interruptAlrm[alrmnum]) {
//    wave.stop();
//    return; // Alarm was interrupted with button, exit
//  }
  char myString[11];
  snprintf(myString,sizeof(myString), "ALRM%d.WAV",Settings.alarmTone[alrmnum]); // Make Alarm Filename
  playalarmfile(myString,alrmnum);
}


// =======================================================================================
// ---- Re-Arm alarm ----
// Automatically re-enables alarm for next time
// By: LensDigital
// =======================================================================================
//void rearmAlrm(byte alrmnum){
//  unsigned long tNow = now();
//  if (Settings.alarmOn[alrmnum] & 128) { // Is global alarm switch on? (1st bit is set?)
//    if ( ( hour(tNow)==Settings.alarmHH[alrmnum]+1) && ( minute(tNow)==Settings.alarmMM[alrmnum]) ) { // It's been 1 hour since Alarm sounded
//      alarmState[alrmnum] = AS_OFF;  // stop sounding alarm
////      if (alrmToneNum[alrmnum]<6) alrmVol[alrmnum]=7; //Set low volume for escalating alarms
//      if (Settings.alarmProgVol[alrmnum]) alrmVol[alrmnum]=ALARM_PROG_STARTVOL; //Set low volume for escalating alarms (wbp)
//      else alrmVol[alrmnum]=0; // Set High volume for non-escalating alarms
//     }
//  }
//}


// =======================================================================================
// ---- Re-Set alarm ----
// Disables alarm for curent day (initiated only by Set button)
// By: LensDigital
// =======================================================================================
boolean resetAlrm(byte alrmnum){
//  if ( (soundAlarm[alrmnum]) || ( (!isInMenu) && snoozeTime[alrmnum]<10)) { // If pressed Stops Alarm for today. If snoozing cancels snooze
  if ( alarmState[alrmnum] >= AS_SOUNDING ) {  // if alarm sounding, snoozing, or skipping
    alarmState[alrmnum] = AS_WAIT;  // stop sounding alarm
    alarmTime[alrmnum] = now();  // set time of state change
//    Serial.print("alarmState: "); Serial.println(alarmState[alrmnum]);
    char string1[20];
    snprintf(string1,sizeof(string1), "Alarm %d Reset", alrmnum+1);
    wave.stop();
//    playcomplete("alrm_res.WAV");
    playfile("alrm_res.WAV");
    scrolltextsizexcolor(4,string1,RED,5, false);  // scroll without checking buttons (wbp)
    delay(500);
    if (Settings.alarmProgVol[alrmnum]) alrmVol[alrmnum]=ALARM_PROG_STARTVOL; //Set low volume for escalating alarms (wbp)
    else alrmVol[alrmnum]=0; // Set High volume for non-escalating alarms
    return true;
    }
  return false;
}


// =======================================================================================
// ---- Skip alarm ----
// Disables pending alarm for curent day (initiated by holding Menu button)
// By: wbp
// =======================================================================================
void skipAlrm(byte alrmnum){
//  Serial.print("skipAlrm: "); Serial.println(alrmnum);
  char string1[20];
  if (alarmState[alrmnum] == AS_SKIPPING) {  // already skipping?
    alarmState[alrmnum] = AS_OFF;  // reset it back to normal
    alarmTime[alrmnum] = now();  // set time of state change
//    Serial.print("alarmState: "); Serial.println(alarmState[alrmnum]);
    snprintf(string1,sizeof(string1), "Alarm %d Reset", alrmnum+1);
    scrolltextsizexcolor(4,string1,RED,5, false);  // scroll without checking buttons (wbp)
  }
  else {
    alarmState[alrmnum] = AS_SKIPPING;
    alarmTime[alrmnum] = now();  // set time of state change
//    Serial.print("alarmState: "); Serial.println(alarmState[alrmnum]);
    snprintf(string1,sizeof(string1), "Alarm %d Skipped", alrmnum+1);
//    wave.stop();
//    playfile("alrm_skip.WAV");
    scrolltextsizexcolor(4,string1,RED,5, false);  // scroll without checking buttons (wbp)
    delay(500);
  }
}


// ============================================================================================================================
// ---- Set alarm Snooze ----
// Handles Snooze logic
// Returns True if Snooze logic was processed. This will tell buttonAction not perform it's primary function (i.e. go to menu)
// By: LensDigital
// ============================================================================================================================
void snoozeProc(byte alrmnum){
  if (alarmState[alrmnum] != AS_SOUNDING)  return; // Alarm wasn't sounding so we are not doing snoozing
//  Serial.print("snooze: "); Serial.println(alrmnum);
  alarmState[alrmnum] = AS_SNOOZING;  // change alarm state to snoozing
  alarmTime[alrmnum] = now();  // remember when Snooze button was pressed
//  Serial.print("alarmState: "); Serial.println(alarmState[alrmnum]);
  wave.stop();
  isInQMenu=false;
  delay(500);
//  cls();
  char myString[15];
  snprintf(myString,sizeof(myString), "Snoozing...");
  scrolltextsizexcolor(4,myString,RED,5,false);  // scroll without checking buttons (wbp)
  delay(500);
}


