// =======================================================================================
// ---- Process Alarm Function ----
// By: LensDigital
// =======================================================================================
void procAlarm(byte alrmnum) {
  if (isInMenu) return; // Do not sound alarm if changing settings
    if (digitalRead(SET_BUTTON_PIN) == HIGH) processSetButton(); // Poll Set Button, that will reArm (cancel for today) alarm
    if (alarmon[alrmnum] & 128){ // Is global alarm switch on? (1st byte is set)
      // ==== Begin alarm LED indicator ====
      if ( snoozeTime[alrmnum]==10) {
        if ( alarmon[alrmnum] & weekdays[weekday()] ) plot (29*alrmnum+1,15,RED); // Show dot in Orange, indicating that alarm is set for current day
        else   plot (29*alrmnum+1,15,GREEN); // No alarm today so show green dot in lower right corner 
      } 
      else plot (29*alrmnum+1,15,ORANGE); // Show dot in RED if snoozing (Maybe do blinking in the future too)
      // ==== END alarm LED indicator ====
      
      if (soundAlarm[alrmnum]) playAlarm(alrmnum);
      // ==== Begin Snooze Check ====
      else {
        // Are We Snoozing?
        if ( snoozeTime[alrmnum]!=10 ) { // Snooze was pressed
         // Is it time reset Snooze?
         if ( (minute()%10) == snoozeTime[alrmnum]) {
           soundAlarm[alrmnum]=true; // Check last digit of current minute
           if (alrmToneNum[alrmnum]<6) alrmVol[alrmnum]=7; // Reset Alarm Volume
           else alrmVol[alrmnum]=0;
         }
      }
       // ==== End Snooze Check ====
      // Maybe it's time to sound alarm for the first time at specified time?
      else
       if (!interruptAlrm[alrmnum]) // Has not been interrupted, takes care if alarm was canceled within 1st minute
         /*
         if ( (alarmon[alrmnum] == 2) && ( (weekday() == 1) || (weekday () == 7) ) ) ;// Do nothing, cause it's weekend and alarm was set to weekday
         else // It's Either Daily alarm or we are in Mon-Fri range
         */
         if ( alarmon[alrmnum] & weekdays[weekday()] ) // Alarm is scheduled for this day!
          if ( (hour()==alrmHH[alrmnum]) && ( minute()==alrmMM[alrmnum])  ) {
            if (alrmnum==0) { // It's first alarm processor
              soundAlarm[1]=false; //Innterrupts 2nd alarm if it's playing
              snoozeTime[1]=10; // Disable snooze for 2nd alarm
            }
            else { // It's first alarm processor
              soundAlarm[0]=false; //Innterrupts 1st alarm if it's playing
              snoozeTime[0]=10; // Disable snooze for 1st alarm
            }
           soundAlarm[alrmnum]=true; // If it's time to sound alarm!
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
  if (alarmon[alrmnum] & 128) { // Is global alarm switch on? (1st bit is set?)
    if ( ( hour()==alrmHH[alrmnum]+1) && ( minute()==alrmMM[alrmnum]) ) { // It's been 1 hour since Alarm sounded
      soundAlarm[alrmnum]=false;
      interruptAlrm[alrmnum]=false;
      if (alrmToneNum[alrmnum]<6) alrmVol[alrmnum]=7; //Set low volume for escalating alarms
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
    if (alrmToneNum[alrmnum]<6) alrmVol[alrmnum]=7; //Set low volume for escalating alarms
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
  if ((minute()%10)==0) snoozeTime[alrmnum]=9;
  else
  snoozeTime[alrmnum]=(minute()%10)-1;
  soundAlarm[alrmnum]=false; 
  interruptAlrm[alrmnum]=true;
  wave.stop();
  interruptAlrm[alrmnum]=false;
  isInQMenu=false;
  delay(1000);

}


