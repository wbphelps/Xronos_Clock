// =======================================================================================
// ---- Process Alarm Function ----
// By: LensDigital
// =======================================================================================
void procAlarm(byte alrmnum) {
  int blinkDotDuration = 1000; // How frequently dots should blink (wbp)
  int iAlrm, iNow, wd;
  unsigned long tNow = now();
  if (isInMenu) return; // Do not sound alarm if changing settings
  if (digitalRead(SET_BUTTON_PIN) == HIGH) processSetButton(); // Poll Set Button, that will reArm (cancel for today) alarm
  if (alarmon[alrmnum] & 128) { // Is global alarm switch on? (1st byte is set)
    // ==== Begin alarm LED indicator ====
    if (soundAlarm[alrmnum]) { // Alarm currently sounding?
      if ( (millis()-alarmBlinkTime > blinkDotDuration)) { // It's been over blinkDuration time
        alarmBlinkTime = millis(); // reset offset to current time
        if ( alarmColor == BLACK )  alarmColor=RED; // Invert color of indicator
        else alarmColor = BLACK;
      }
      plot (31*alrmnum,14,alarmColor); // Show blinking dot in Orange if snoozing  (wbp)
    }
    else if ( snoozeTime[alrmnum]==10 ) { // not snoozing?

//      if ( alarmon[alrmnum] & weekdays[weekday()] )  // Is alarm on and set for today?
//        plot (31*alrmnum,14,RED); // Show dot in Red, indicating that alarm is set for current day  (wbp)
//      else
//        plot (31*alrmnum,14,GREEN); // No alarm today so show green dot in lower left(right) corner (wbp)

      // check to see if alarm will sound again within 24 hours
      alarmColor=RED;  // assume it won't
      wd = weekday(tNow);  // today's weekday number
      iAlrm = alrmHH[alrmnum]*60 + alrmMM[alrmnum];  // time of alarm in minutes
      iNow = hour(tNow)*60 + minute(tNow);  // time now in minutes
      if (iNow>iAlrm) { // is tAlrm in the past?
        iAlrm+=1440;  // set tAlrm ahead by one day
        wd+=1;  // let's look at tomorrow
        if (wd>7)  wd=1;  // wrap if necessary
      }
      if (alarmon[alrmnum] & weekdays[wd]) {  // is alarm on and set for the day in question?
        if ((iAlrm-iNow)<=1440)  // is alarm set to go off in next 24 hours?
          alarmColor=GREEN;  // change LED to red
      }
      plot (31*alrmnum,14,alarmColor); // show alarm status
      
    } 

    else  { // snoozing
      if ( (millis()-alarmBlinkTime > blinkDotDuration)) { // It's been over blinkDuration time
        alarmBlinkTime = millis(); // reset offset to current time
        if ( alarmColor == BLACK )  alarmColor=ORANGE; // Invert color of indicator
        else alarmColor = BLACK;
      }
      plot (31*alrmnum,14,alarmColor); // Show blinking dot in Orange if snoozing  (wbp)
    }
    // ==== END alarm LED indicator ====
      
    if (soundAlarm[alrmnum]) {  // already sounding alarm?
      playAlarm(alrmnum);
    }
    // ==== Begin Snooze Check ====
    else {
      // Are We Snoozing?
      if ( snoozeTime[alrmnum]!=10 ) { // Snooze was pressed
        // Is it time reset Snooze?
        if ( (minute(tNow)%10) == snoozeTime[alrmnum]) {
          soundAlarm[alrmnum]=true; // Check last digit of current minute
//        if (alrmToneNum[alrmnum]<=ALARM_PROGRESSIVE)  alrmVol[alrmnum]=7; // Reset Alarm Volume
          if (alrmProgVol[alrmnum])  alrmVol[alrmnum]=7; // Reset Alarm Volume
          else  alrmVol[alrmnum]=0;
        }
      }
      // ==== End Snooze Check ====
      // Maybe it's time to sound alarm for the first time at specified time?
      else if (!interruptAlrm[alrmnum]) { // Has not been interrupted, takes care if alarm was canceled within 1st minute
         /*
        if ( (alarmon[alrmnum] == 2) && ( (weekday() == 1) || (weekday () == 7) ) ) ;// Do nothing, cause it's weekend and alarm was set to weekday
        else // It's Either Daily alarm or we are in Mon-Fri range
         */
          if ( alarmon[alrmnum] & weekdays[weekday()] ) { // Alarm is scheduled for this day!
            if ( (hour(tNow)==alrmHH[alrmnum]) && ( minute(tNow)==alrmMM[alrmnum]) ) {
              if (alrmnum==0) { // It's first alarm processor
                soundAlarm[1]=false; // Interrupts 2nd alarm if it's playing
                snoozeTime[1]=10; // Disable snooze for 2nd alarm
              }
            else { // It's second alarm processor
              soundAlarm[0]=false; // Interrupts 1st alarm if it's playing
              snoozeTime[0]=10; // Disable snooze for 1st alarm
            }
            soundAlarm[alrmnum]=true; // If it's time to sound alarm!
            }
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
  turnOffRadio(); // Disable RF12B
  if (interruptAlrm[alrmnum]) {
      wave.stop();
     return; // Alarm was interrupted with button, exit
  }
  char myString[11];
  snprintf(myString,sizeof(myString), "ALRM%d.WAV",alrmToneNum[alrmnum]); // Make Alarm Filename
  playalarmfile(myString,alrmnum);
  
}


// =======================================================================================
// ---- Re-Arm alarm ----
// Automatically re-enables alarm for next time
// By: LensDigital
// =======================================================================================
void rearmAlrm(byte alrmnum){
  unsigned long tNow = now();
  if (alarmon[alrmnum] & 128) { // Is global alarm switch on? (1st bit is set?)
    if ( ( hour(tNow)==alrmHH[alrmnum]+1) && ( minute(tNow)==alrmMM[alrmnum]) ) { // It's been 1 hour since Alarm sounded
      soundAlarm[alrmnum]=false;
      interruptAlrm[alrmnum]=false;
//      if (alrmToneNum[alrmnum]<6) alrmVol[alrmnum]=7; //Set low volume for escalating alarms
      if (alrmProgVol[alrmnum]) alrmVol[alrmnum]=7; //Set low volume for escalating alarms (wbp)
      else alrmVol[alrmnum]=0; // Set High volume for non-escalating alarms
      snoozeTime[alrmnum]=10; // Turn off snooze
     }
  }
}



// =======================================================================================
// ---- Re-Set alarm ----
// Disables alarm for curent day (initiated only by Set button)
// By: LensDigital
// =======================================================================================
boolean resetAlrm(byte alrmnum){
  //if (alarmon[alrmnum] & 128){ // Is global alarm switch on?  (1st bit is set?)
  if ( (soundAlarm[alrmnum]) || ( (!isInMenu) && snoozeTime[alrmnum]<10)) { // If pressed Stops Alarm for today. If snoozing cancelss snooze
    interruptAlrm[alrmnum]=true;
    soundAlarm[alrmnum]=false;
    snoozeTime[alrmnum]=10;
    wave.stop();
    playcomplete("alrm_res.WAV");
    if (alrmProgVol[alrmnum]) alrmVol[alrmnum]=7; //Set low volume for escalating alarms (wbp)
    else alrmVol[alrmnum]=0; // Set High volume for non-escalating alarms
    return true;
    }
    return false;
}

// ============================================================================================================================
// ---- Set alarm Snooze ----
// Handles Snooze logic
// Returns True if Snooze logic was processed. This will tell buttonAction not perform it's primary function (i.e. go to menu)
// By: LensDigital
// ============================================================================================================================
void snoozeProc(byte alrmnum){
  if (!soundAlarm[alrmnum]) return; // Alarm wasn't sounding so we are not doing snoozing
  //Serial.println ("Snooze was pressed!");
  if ((minute()%10)==0)  snoozeTime[alrmnum]=9;
  else  snoozeTime[alrmnum]=(minute()%10)-1;
  soundAlarm[alrmnum]=false; 
  interruptAlrm[alrmnum]=true;
  wave.stop();
  interruptAlrm[alrmnum]=false;
  isInQMenu=false;
  delay(1000);
}


