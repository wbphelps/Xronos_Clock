// =======================================================================================
// ---- Process Alarm Function ----
// By: LensDigital
// =======================================================================================
//static unsigned long alarmBlinkTime=0; // controls blinking of alarm indicators
void procAlarm(byte alrmnum) {
  int blinkDuration = 1000; // How frequently dots should blink (wbp)
  int iAlrm, iNow, wd;
  unsigned long tNow = now();
  if (isInMenu) return; // Do not sound alarm if changing settings
///  if (digitalRead(SET_BUTTON_PIN) == HIGH) processSetButton(); // Poll Set Button, that will reArm (cancel for today) alarm  ???
  if (Settings.alarmOn[alrmnum] & 128) { // Is global alarm switch on? (1st bit is set)

// ==== Begin alarm LED indicator ====
//    if (millis()-alarmBlinkTime >= blinkDuration) { // run this part once per second
//      alarmBlinkTime = millis(); // reset offset to current time

      if (soundAlarm[alrmnum]) { // Alarm currently sounding?
//        if ( alarmColor == BLACK )  alarmColor=RED; // Invert color of indicator
//        else alarmColor = BLACK;
        if (blinkOn)  alarmColor=RED;  // blink alarm indicator
        else alarmColor=BLACK;
        plot (31*alrmnum,14,alarmColor); // Show blinking dot in Orange if snoozing  (wbp)
      }

      else if ( snoozeTime[alrmnum] < 10)  { // snoozing
//        if ( alarmColor == BLACK )  alarmColor=ORANGE; // Invert color of indicator
//        else alarmColor = BLACK;
        if (blinkOn)  alarmColor=ORANGE;  // use Yellow for snoozing alarm
        else alarmColor=BLACK;  // and blink it
        plot (31*alrmnum,14,alarmColor); // Show blinking dot in Orange if snoozing  (wbp)
      }

      else  { // not sounding and not snoozing...
        // check to see if alarm will sound again within 24 hours
        alarmColor=RED;  // assume it won't
        wd = weekday(tNow);  // today's weekday number
        iAlrm = Settings.alarmHH[alrmnum]*60 + Settings.alarmMM[alrmnum];  // time of alarm in minutes
        iNow = hour(tNow)*60 + minute(tNow);  // time now in minutes
        if (iNow>iAlrm) { // is tAlrm in the past?
          iAlrm+=1440;  // set tAlrm ahead by one day
          wd+=1;  // let's look at tomorrow
          if (wd>7)  wd=1;  // wrap if necessary
        }
        if (Settings.alarmOn[alrmnum] & weekdays[wd]) {  // is alarm on and set for the day in question?
          if ((iAlrm-iNow)<=1440)  // is alarm set to go off in next 24 hours?
            alarmColor=GREEN;  // change LED to red
        }
        plot (alrmnum*31,14,alarmColor); // show alarm status
      } 
//  }
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
          if (Settings.alarmProgVol[alrmnum])  alrmVol[alrmnum]=ALARM_PROG_STARTVOL; // Reset Alarm Volume
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
          if ( Settings.alarmOn[alrmnum] & weekdays[weekday()] ) { // Alarm is scheduled for this day!
            if ( (hour(tNow)==Settings.alarmHH[alrmnum]) && ( minute(tNow)==Settings.alarmMM[alrmnum]) ) {
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
  snprintf(myString,sizeof(myString), "ALRM%d.WAV",Settings.alarmTone[alrmnum]); // Make Alarm Filename
  playalarmfile(myString,alrmnum);
  
}


// =======================================================================================
// ---- Re-Arm alarm ----
// Automatically re-enables alarm for next time
// By: LensDigital
// =======================================================================================
void rearmAlrm(byte alrmnum){
  unsigned long tNow = now();
  if (Settings.alarmOn[alrmnum] & 128) { // Is global alarm switch on? (1st bit is set?)
    if ( ( hour(tNow)==Settings.alarmHH[alrmnum]+1) && ( minute(tNow)==Settings.alarmMM[alrmnum]) ) { // It's been 1 hour since Alarm sounded
      soundAlarm[alrmnum]=false;
      interruptAlrm[alrmnum]=false;
//      if (alrmToneNum[alrmnum]<6) alrmVol[alrmnum]=7; //Set low volume for escalating alarms
      if (Settings.alarmProgVol[alrmnum]) alrmVol[alrmnum]=ALARM_PROG_STARTVOL; //Set low volume for escalating alarms (wbp)
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
  if ( (soundAlarm[alrmnum]) || ( (!isInMenu) && snoozeTime[alrmnum]<10)) { // If pressed Stops Alarm for today. If snoozing cancelss snooze
    interruptAlrm[alrmnum]=true;
    soundAlarm[alrmnum]=false;
    snoozeTime[alrmnum]=10;  // reset snooze
    char string1[15];
    snprintf(string1,sizeof(string1), "Alarm Reset");
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
  delay(500);
//  cls();
  char myString[15];
  snprintf(myString,sizeof(myString), "Snoozing...");
  scrolltextsizexcolor(4,myString,RED,5,false);  // scroll without checking buttons (wbp)
  delay(500);
}


