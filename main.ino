// =======================================================================================
// ---- First time clock is powered, play startup music, show vers and say time ----
// ---------------------------------------------------------------------------------------
void startup ()
{
  if (!doStartup) return; // Startup Disabled?
  //void initEPROM();
  char welcome[15];
  byte ver=EEPROM.read (clockVerLoc); // Read 3 digit version number
//  byte temp = (ver%100) %10; //temp holder
  byte temp = (ver%100); //temp holder (wbp)
  byte ver3 = temp % 10; // Last digit
  byte ver2 = (temp - ver3) / 10; // Second Digit
  ver = (ver - ver2) / 100; // First digitf
  playfile("startup1.wav");
  showText(0,0,"Welcome",3,GREEN);
  snprintf(welcome, sizeof(welcome),"Version:%d.%d%dWm",ver,ver2,ver3);  // wbp
  scrolltextsizexcolor(8,welcome,RED,15);
  cls();
  showBigTime(clockColor);
}

// =======================================================================================
// ---- Initializes EEPROM for new Chip. run only first time ----
// ---------------------------------------------------------------------------------------
void initEPROM()
{
  putstring_nl ("Writing Initial settings to EEPROM!!!");
 for (byte i=0;i<2;i++) {
    EEPROM.write (alarmHHLoc[i],0);  
    delay (50);
    EEPROM.write (alarmMMLoc[i],0);
    delay (50);
    EEPROM.write (alarmOnOffLoc[i],0);
    delay (50);
    EEPROM.write(alarmToneLoc[i],1); // Write Alarm Tone number
    delay (50);
 }
  EEPROM.write (mode24HRLoc,false); // Write time mode (12/24 hours)
  delay (50);
  EEPROM.write (brightLoc,3); // Write Brightness setting
  delay (50);
  EEPROM.write (clockColorLoc,GREEN); // Write Clock Color
  delay (50);
  EEPROM.write (clockVerLoc,firmware_ver); 
  delay (50);
  EEPROM.write(clockFontLoc,1); // Write Alarm Tone number
  delay (50);
  EEPROM.write (sFXLoc,1); // Write Menu SFX on/off
  delay (50);
  EEPROM.write (tempUnitLoc,true);// Write Temperature units
  delay (50);
  EEPROM.write (sndVolLoc,0);// Write Sound Volume
  delay (50);
  EEPROM.write(infoFreqLoc,2); // Write infodisplay freq options
  delay (50);
  EEPROM.write(sayOptionsLoc,255); // Write Say options
  delay (50);
  EEPROM.write(doStartupLoc,true); // Write Startup option
  delay (50);
  EEPROM.write(tmpOffsetLoc,2); // Write Temperature Offset
  delay (50);
  EEPROM.write(infoOptionsLoc,2); // Write InfoDisplay options
  delay (50);
  EEPROM.write(radioOnLoc,false); // Disable RFM by default during INIT
  delay (50);
  EEPROM.write(IROnLoc,false); // Disable IR by default during INIT
  delay (50);
  
  //wave.volume=sndVol; // Change System Sound Volume
}

// Display divider colon :
void showDivider(byte color){
  int x=0; //offset
  if(time12hr) x=1; // Offset divider by 2 dots left for 12 hr mode
//  if (clockFont<3) {
    plot (15-x,5,color);
    plot (15-x,6,color);
    plot (15-x,9,color);
    plot (15-x,10,color);
//  }
  plot (16-x,5,color);
  plot (16-x,6,color);
  plot (16-x,9,color);
  plot (16-x,10,color);
}

// =======================================================================================
// ---- Info display ----
// By: LensDigital
// ---------------------------------------------------------------------------------------
void infoDisplay() {
  if (infoFreq == 0) return; // User choose to never show this screen
  if (! ((infoOptions & 128) || (infoOptions & 64) || (infoOptions & 32) || (infoOptions & 16) || (infoOptions & 8) ) ) return; // All display options disabled
  if (soundAlarm[0] || soundAlarm[1]) return; // Do not show if Alarm is playing
  if (isInMenu) return; // Do not show when in menu
  if (isInQMenu) return; // Do not show when in quick menu
  // Check if it's time to Show Info display 
  if ( (minute()%10)%infoFreq == 0 && ( second()==5) )  { // Show date and temp every XX minutes and 5 seconds
    char myString[44];
    //if ( (minute()%10)==0) return; // Prevents showing during 0 digit time
    cls();
    showSmTime(0,clockColor); // Show Time on top
    if (infoOptions & 128) // Option Enabled?
      if (!showDate(clockColor)) return; // Scroll Date. Exit if scroll was interrupted by button press
    if (infoOptions & 64) // Option Enabled?
      if (!showTemp(clockColor,false,true)) return; // Scroll Inside Temp. Exit if scroll was interrupted by button press
    cls(); 
    if (isRadioPresent) {
        if (infoOptions & 32) {// Option Enabled?
          if (!showTemp(clockColor,false,false)) return; // Scroll Outside Temp. Exit if scroll was interrupted by button press
         cls();
        }
        if (infoOptions & 4) {// Humidity Option Enabled?
        if (!showHumidity(clockColor,false)) return; //Scroll Outside Humidity. Exit if scroll was interrupted by button press
        cls ();
        }
    }
    if (infoOptions & 16) {// Option Enabled?
      if (!alarmInfo(0)) return; //Show info for alarm 1. Exit if scroll was interrupted by button press
      cls ();
      if (!alarmInfo(1)) return; //Show info for alarm 2. Exit if scroll was interrupted by button press
      cls();
    }
    if (isRadioPresent) 
      if (infoOptions & 8) {// Option Enabled?
        if ( last_RF > 0 && ( (millis()-last_RF) < 1800000) )  { // Show sensor timestamp
          //Serial.println ("Sensor data receiveed recently");
          showSmTime(0,clockColor); // Show Time on top
          snprintf(myString,sizeof(myString), "Sensor data received %2d seconds ago",second(last_RF));
          if (! scrolltextsizexcolor(8,myString,clockColor,5) ) return;
          cls ();
        }
        else { // Sensor data hasn't been reiceved in a while
          showSmTime(0,clockColor); // Show Time on top
          snprintf(myString,sizeof(myString), "Sensor data not received in over 30 minutes (%2d)",second(last_RF));
          Serial.println (last_RF);
          if (! scrolltextsizexcolor(8,myString,RED,5) ) return;
        }
        cls ();
      }
  }
}

// =======================================================================================
// ---- shows alarm information For Info display ----
// Returns False if scrolling function was interrupted by buttons
// By: LensDigital
// ---------------------------------------------------------------------------------------
boolean alarmInfo(byte alrmNum){
  if ( ! (alarmon[alrmNum] & 128) ) return true; //  Alarm is off
     char myString[54]; // String to keep Alarm msg
     char wkdays[28]; //Days of week
     showSmTime(0,clockColor); // Show time on top
     if ( snoozeTime[alrmNum]!=10 ) { // Are we snoozing?
       snprintf(myString,sizeof(myString), "Alarm%d on! Snoozing...Z Z z z z z...",alrmNum+1);
       return scrolltextsizexcolor(8,myString,RED,5);
     }
     else { // Alarm is on but not snoozing
       if (alarmon[alrmNum] == DAILY) {  // Daily Alarm
         if(time12hr) { // It's 12 hour mode
           if (hoursMode(alrmHH[alrmNum]) )  // Check if it's AM
             snprintf(myString,sizeof(myString), "Alarm %d is set Daily at %d:%02d AM",alrmNum+1,myhours,alrmMM[alrmNum]); 
           else // It's PM
             snprintf(myString,sizeof(myString), "Alarm %d is set Daily at %d:%02d PM",alrmNum+1,myhours,alrmMM[alrmNum]); 
         }
         else // It's 24 hour mode
           snprintf(myString,sizeof(myString), "Alarm %d is set Daily at %02d:%02d",alrmNum+1,alrmHH[alrmNum],alrmMM[alrmNum]); 
       }
       else if (alarmon[alrmNum] == WEEKDAY) {  // It's WeekDay alarm
         if(time12hr) { // It's 12 hour mode
           myhours = alrmHH[alrmNum];
           if (myhours == 0) 
             myhours=12; // It's Midnight
           else if (myhours >12) myhours=myhours-12;
           if (alrmHH[alrmNum]<12) // Show AM
             snprintf(myString,sizeof(myString), "Alarm %d is set M-F at %d:%02d AM",alrmNum+1,myhours,alrmMM[alrmNum]); 
           else 
             snprintf(myString,sizeof(myString), "Alarm %d is set M-F at %d:%02d PM",alrmNum+1,myhours,alrmMM[alrmNum]); 
         }
         else // It's 24 hour mode
         snprintf(myString,sizeof(myString), "Alarm %d is set M-F at %02d:%02d",alrmNum+1,alrmHH[alrmNum],alrmMM[alrmNum]); 
       }
       else {  // It's Custom alarm
          // Make custom string containing each weekday that alarm is set too
         strcpy (wkdays,"");
         if (alarmon[alrmNum] & MON)  strcat (wkdays, "Mon ");
         if (alarmon[alrmNum] & TUE)  strcat (wkdays, "Tue ");
         if (alarmon[alrmNum] & WED)  strcat (wkdays, "Wed ");
         if (alarmon[alrmNum] & THU)  strcat (wkdays, "Thu ");
         if (alarmon[alrmNum] & FRI)  strcat (wkdays, "Fri ");
         if (alarmon[alrmNum] & SAT)  strcat (wkdays, "Sat ");
         if (alarmon[alrmNum] & SUN)  strcat (wkdays, "Sun ");
           
          if(time12hr) { // It's 12 hour mode
           myhours = alrmHH[alrmNum];
           if (myhours == 0) 
             myhours=12; // It's Midnight
           else if (myhours >12) myhours=myhours-12;
           if (alrmHH[alrmNum]<12) // Show AM
             snprintf(myString,sizeof(myString), "Alarm%d is set to %d:%02d AM %s",alrmNum+1,myhours,alrmMM[alrmNum],wkdays); 
           else 
             snprintf(myString,sizeof(myString), "Alarm%d is set to %d:%02d PM %s",alrmNum+1,myhours,alrmMM[alrmNum],wkdays); 
         }
         else // It's 24 hour mode
         snprintf(myString,sizeof(myString), "Alarm%d is set to %02d:%02d %s",alrmNum+1,alrmHH[alrmNum],alrmMM[alrmNum],wkdays); 
       }
     return scrolltextsizexcolor(8,myString,clockColor,5);
     }
     
}


// =======================================================================================
// ---- Says current time ----
// By: LensDigital
// ---------------------------------------------------------------------------------------
void sayTime(){
     playcomplete("TIME_IS.WAV");
     char myString[9];
     if(time12hr) { // == 12 Hour Mode ====
       snprintf(myString,sizeof(myString), "%d.WAV",hourFormat12()); // Make Hours string
       playcomplete(myString); // Play Hours
     }
     else // == 24 Hour Mode ====
       if (hour() > 20) { // Make complex 2 digit sound
         playcomplete("20.WAV"); // Play Hours 1st digit
         snprintf(myString,sizeof(myString), "%d.WAV",hour()%10); // Make Hours 2nd digit string
         playcomplete(myString); // Play Hours 2nd digit
       }
       else { // Simple
         snprintf(myString,sizeof(myString), "%d.WAV",hour()); // Make Hours string
         playcomplete(myString); // Play Hours
       }
     if (minute()<20) {
       if (minute()==0) { playcomplete("100.WAV"); } // We are at hh:00
       else {
         if (( (minute()/10)%10) == 0) playcomplete ("OH.WAV"); // If first digit of minute is 0, say "oh"
         snprintf(myString,sizeof(myString), "%d.WAV",minute()); // Make Minutes string
         playcomplete(myString);
         }
       }
     else { // Make complex 2 digit sound
       minutes=(minute()/10)%10;
       switch (minutes){ // Create 1st digit sound
         case 2:
           playcomplete ("20.WAV");
           break;
         case 3:
           playcomplete ("30.WAV");
           break;
         case 4:
           playcomplete ("40.WAV");
           break;
         case 5:
           playcomplete ("50.WAV");
           break;
       }
       if ((minute()%10)!=0) { // Don't say if last digit is 0
         snprintf(myString,sizeof(myString), "%d.WAV",minute()%10); // Make 2nd digit
         playcomplete(myString); // Play 
       }
     }
     // If needed say AM or PM
     if(time12hr) { // == 12 Hour Mode ====
       if (isAM() ) playcomplete("AM.WAV");
       else playcomplete("PM.WAV");
     }
   
}

// =======================================================================================
// ---- Says current time ----
// By: LensDigital
// ---------------------------------------------------------------------------------------
void sayAlarm(byte alrmNum){
  char myString[9];  
   playcomplete("ALARM.WAV");
  snprintf(myString,sizeof(myString), "%d.WAV",alrmNum+1); // Make and say alarm number
  playcomplete(myString);
  playcomplete("IS.WAV");
  playcomplete("SET.WAV");
  playcomplete("2.WAV");
  
  if(time12hr) { // It's 12 hour mode
           myhours = alrmHH[alrmNum];
           if (myhours == 0) 
             myhours=12; // It's Midnight
           else if (myhours >12) myhours=myhours-12;
           snprintf(myString,sizeof(myString), "%d.WAV",myhours); // Make hours filename
           playcomplete(myString); // Play Hours
   }
     else // == 24 Hour Mode ====
       if (alrmHH[alrmNum] > 20) { // Make complex 2 digit sound
         playcomplete("20.WAV"); // Play Hours 1st digit
         snprintf(myString,sizeof(myString), "%d.WAV",alrmHH[alrmNum]%10); // Make Hours 2nd digit string
         playcomplete(myString); // Play Hours 2nd digit
       }
       else { // Simple
         snprintf(myString,sizeof(myString), "%d.WAV",alrmHH[alrmNum]); // Make Hours string
         playcomplete(myString); // Play Hours
       }
     if (alrmMM[alrmNum]<20) {
       if (alrmMM[alrmNum]==0) { playcomplete("100.WAV"); } // We are at hh:00
       else {
         if (( (alrmMM[alrmNum]/10)%10) == 0) playcomplete ("OH.WAV"); // If first digit of minute is 0, say "oh"
         snprintf(myString,sizeof(myString), "%d.WAV",alrmMM[alrmNum]); // Make Minutes string
         playcomplete(myString);
         }
       }
     else { // Make complex 2 digit sound
       minutes=(alrmMM[alrmNum]/10)%10;
       switch (minutes){ // Create 1st digit sound
         case 2:
           playcomplete ("20.WAV");
           break;
         case 3:
           playcomplete ("30.WAV");
           break;
         case 4:
           playcomplete ("40.WAV");
           break;
         case 5:
           playcomplete ("50.WAV");
           break;
       }
       if ((alrmMM[alrmNum]%10)!=0) { // Don't say if last digit is 0
         snprintf(myString,sizeof(myString), "%d.WAV",alrmMM[alrmNum]%10); // Make 2nd digit
         playcomplete(myString); // Play 
       }
     }
     // If needed say AM or PM
     if(time12hr) { // == 12 Hour Mode ====
       if (alrmHH[alrmNum]<12)  playcomplete("AM.WAV");
       else playcomplete("PM.WAV");
     }
     if (alarmon[alrmNum] == DAILY) playcomplete("DAILY.WAV");// Daily Alarm
     else if (alarmon[alrmNum] == WEEKDAY) playcomplete("wkdays.WAV");// Workday Alarm
     else { // Custom ALarm
       playcomplete("EVERY.WAV");
       if (alarmon[alrmNum] & MON) playcomplete("MON.WAV");
       if (alarmon[alrmNum] & TUE) playcomplete("TUE.WAV");
       if (alarmon[alrmNum] & WED) playcomplete("WED.WAV");
       if (alarmon[alrmNum] & THU) playcomplete("THU.WAV");
       if (alarmon[alrmNum] & FRI) playcomplete("FRI.WAV");
       if (alarmon[alrmNum] & SAT) playcomplete("SAT.WAV");
       if (alarmon[alrmNum] & SUN) playcomplete("SUN.WAV");
     }
     
}

// =======================================================================================
// ---- Announce Internal Temperature ----
// By: LensDigital
// ---------------------------------------------------------------------------------------
void sayTemp(int temp, boolean location){
  /*
  if (temp > 100 || temp < -50) {
    playcomplete ("ERR1.WAV");
    return;
  } */
  char myString[8];
  playcomplete("TEMP.WAV");
  if (location) playcomplete("INSIDE.WAV");
  else playcomplete("OUTSIDE.WAV");
  playcomplete("IS.WAV");
  
  snprintf(myString,sizeof(myString), "%d0.WAV",(temp/10)%10); // first digit
  playcomplete(myString);
  if ((temp%10)!=0) {
    snprintf(myString,sizeof(myString), "%d.WAV",temp%10); // first digit
    playcomplete(myString);
  }
  playcomplete("DEGREES.WAV");
  if (tempUnit) playcomplete("fahrenh.WAV");
  else playcomplete("celcius.WAV");
}


// =======================================================================================
// ---- Announce Current Date ----
// By: LensDigital
// ---------------------------------------------------------------------------------------
void sayDate(){
    char myString[10];
  char dayofweek[4]; // Keeps day of week
  playcomplete("TODAY.WAV");
  playcomplete("IS.WAV");
  Serial.println ("Say Date!");
  switch (weekday()){
    case 1: //Sunday
    strcpy(dayofweek,"SUN");
    break;
    case 2: //Mon
    strcpy(dayofweek,"MON");
    break;
    case 3: 
    strcpy(dayofweek,"TUE");
    break;
    case 4: 
    strcpy(dayofweek,"WED");
    break;
    case 5: 
    strcpy(dayofweek,"THU");
    break;
    case 6: 
    strcpy(dayofweek,"FRI");
    break;
    case 7: 
    strcpy(dayofweek,"SAT");
    break;
  }
  snprintf(myString,sizeof(myString), "%s.WAV",dayofweek); // Day of Week
  playcomplete(myString);
  snprintf(myString,sizeof(myString), "%s.WAV",monthShortStr(month()) ); // month
  playcomplete(myString);
  if (day()<20) {
      snprintf(myString,sizeof(myString), "%dst.WAV",day()); // Make Day String
      playcomplete(myString);
         
  }
  else { // Make complex 2 digit sound
       days=(day()/10)%10;
       switch (days){ // Create 1st digit sound
         case 2:
           playcomplete ("20.WAV");
           break;
         case 3:
           playcomplete ("30.WAV");
           break;
       }
       if ((day()%10)!=0) { // Don't say if last digit is 0
         snprintf(myString,sizeof(myString),"%dst.WAV",day()%10); // Make 2nd digit
         playcomplete(myString); // Play 
       }
     }
  playcomplete("2.WAV");
  playcomplete("1000.WAV");
  snprintf(myString,sizeof(myString), "%d.WAV",year()%100); // Make 2nd digit
  playcomplete(myString); // Play 
}


// =======================================================================================
// Display large digit (full screen) time
// By: LensDigital
// ---------------------------------------------------------------------------------------
void showBigTime(byte color){
  //if (menuItem > 3 || !okClock) return; // Date setting is in progress. Do not show clock
  if (!okClock) return; // Date setting is in progress. Do not show clock
  int blinkDotDuration = 1000; // How frequently dots should blink (wbp)
  int blinkDigDuration = 500; // Frequencey of digit blinking during time setting
  int x=0; //offset
  if (color==4)color=random(3)+1; // Select random color
  
  // Blinker processor (used to blink divider and/or digits during time setting
  if (!isInMenu)  // If we are setting time, don't blick divider
    if ( (millis()-blinkTime > blinkDotDuration)) { // It's been over blinkDuration time
      blinkTime = millis(); // reset offset to current time
      if ( blinkColor == BLACK )  blinkColor=color; // Inverse color of divider
      else blinkColor = BLACK;
      //else if (!blinking) blinkColor=BLACK; // If setting time don't blink divider
    }  
  
  if (blinking) { // Setting time, so blink correct digits
    //Serial.println ("Setting time");
    blinkColor=color; // Show steady divider dots
    if ( (millis()-blinkTime > blinkDigDuration)) { // It's been over blinkDuration time
      blinkTime = millis(); // reset offset to current time
      if (isSettingHours) {
        //putstring_nl ("Setting hours");
       if ( hhColor == BLACK ) hhColor = color; // Inverse color of Hours 
       else hhColor = BLACK;
       mmColor = color; // Minutes not blinking
      }
      if (isSettingMinutes) {
        //putstring_nl ("Setting minutes");
       if ( mmColor == BLACK ) mmColor = color; // Inverse color of Minutes 
       else mmColor = BLACK;
       hhColor = color; // Hours not blinking
      }
    }
  }
  else { hhColor=color; mmColor=color;} // We are not setting time, so show digits as usual
  // --- END OF BLINK PROCESSOR
    
  // Check if we are running in 12 Hour Mode:
  if(time12hr) {
  // == BEGIN 12 Hour Mode ====
    x=2; //offset hours by 2 dots
    myhours=hourFormat12();
//    if (isAM()) plot (0,1,hhColor); // Show AM Dot
    if (isPM()) plot (0,1,hhColor); // Show Dot for PM
    else plot (0,1,BLACK); // Hide Dot
    if ( (myhours/10)%10 == 0 ) showDigit(0-x,2,1,5,clockFont,BLACK); // Hide first digit 
    else showDigit(0-x,2,(myhours/10)%10,5,clockFont,hhColor);
  }
  // === END 12 Hour Mode ===
  else {
  // 24 Hour Mode
   myhours=hour();
    //plot (0,1,BLACK); // Hide PM Dot (wbp)
    showDigit(0,2,(hour()/10)%10,5,clockFont,hhColor); // Show 1st digit of hour
  }
  showDigit(6-x,2,myhours%10,5,clockFont,hhColor); // Always Show 2nd digit of hour
  showDivider (blinkColor);
  showDigit(15,2,(minute()/10)%10,5,clockFont,mmColor); // Show 1st digit of minute
  showDigit(22,2,minute()%10,5,clockFont,mmColor); // Show 2nd digit of minute
  
}


// =======================================================================================
// ---- Display smaller digit time ----
// By: LensDigital
// Location is either 0: top row, or 1: buttom row
// ---------------------------------------------------------------------------------------
void showSmTime (byte location,byte color){
  char myString[6];
  location=location*8; // Shift to bottom row if 1
  // Check if we are running in 12 Hour Mode:
  if(time12hr) {
  // == BEGIN 12 Hour Mode ====
    myhours=hourFormat12();
//    if (isAM()) plot (0,1,color); // Show AM Dot
    if (isPM()) plot (0,1,color); // Show Dot for PM
    else plot (0,1,BLACK); // Hide Dot
    snprintf(myString,sizeof(myString), "%d",myhours); // Make hour string
    if ( (myhours/10)%10 == 0 )  // It's one digit hour so need to shift it to the right
      showText(7,location,myString,1,color); // Shift hour to the right since it's sigle digit
    else 
      showText(1,location,myString,1,color); // Show hour at normal position
    
  }
  // === END 12 Hour Mode ===
  else {
  // 24 Hour Mode
   snprintf(myString,sizeof(myString), "%02d",hour()); // make 2 digit hours
   showText(1,location,myString,1,color);
    //plot (0,1,BLACK); // Hide PM Dot
  }
  snprintf(myString,sizeof(myString), "%02d",minute());
  showText(12,location,":",1,color); // Show colum :
  showText(18,location,myString,1,color); // Show minutes
  
  
}

// =======================================================================================
// ---- Display Date and year (mostly for setting) ----
// By: LensDigital
// ---------------------------------------------------------------------------------------
void mainDate(byte color){
  if (!isSettingDate) return; // Not setting Date
  int blinkDigDuration = 500; // Frequencey of digit blinking during time setting
  char dateString[5]; // Var will hold generated string
  if (color==4)color=random(3)+1; // Select random color
  // Blinker processor (used to blink text, number during date setting
  if (isSettingTime){
     if ( (millis()-blinkTime > blinkDigDuration)) { // It's been over blinkDuration time
      blinkTime = millis(); // reset offset to current time
      if (blinking){ // Setting date, so blink correct string
         if ( dateColor == BLACK ) dateColor=color;
         else  dateColor=BLACK;
      }
      else dateColor=color;
    }  
    if (isSettingMonth) {
       monColor=dateColor; 
       ddColor=color; 
       yyColor=color;
     }
     if (isSettingDay) {
       monColor=color;
       ddColor=dateColor;
       yyColor=color;
     }
     if (isSettingYear) {
       monColor=color;
       ddColor=color;
       yyColor=dateColor;
     }
  }
  else {
    monColor=color;
    ddColor=color;
    yyColor=color;
  }
  // --- END OF BLINK PROCESSOR
   
   snprintf(dateString, sizeof(dateString),"%s",monthShortStr(month()) ); // Create Month String
   showText(1,0,dateString,2,monColor); // Show month
   snprintf(dateString, sizeof(dateString),"%2d",day() ); // create Day strng
   showText(20,0,dateString,2,ddColor); // Show day
   snprintf(dateString, sizeof(dateString),"%2d",(year()) ); // create Year String
   showText(5,8,dateString,1,yyColor); // Show year
}
// ---------------------------------------------------------------------------------------


// =======================================================================================
// ---- Display Current Temperature  ----
// ---- speak = true if voice annoucment desired
// ---- location = true for inside, false for outside
// By: LensDigital
// =======================================================================================
boolean showTemp(byte color,boolean speak, boolean location){
  char myString[32];
  char tempInOut[7];
  byte tmpOffset2=0; // will differ if getting temp from outside
  if (location)  tmpOffset2=tmpOffset; //Only for Inside temp Actual offset is used
  boolean returnVal=true;// Return value
  float tempC;
  if (location) { // Get temperature from attached sensor
    sensors.requestTemperatures(); // Send the command to get temperatures
    tempC = sensors.getTempC(insideThermometer);
    snprintf(tempInOut,sizeof(tempInOut), "In");
  }
  else { // Get Temperature from external sensor
    if (!isRadioPresent) return true;
    tempC=extTemp; // External Temperature was requested
    snprintf(tempInOut,sizeof(tempInOut), "Out");
  }
  if (tempC > 100 || tempC < -50) { // Temperature sensor is not working
    //playSFX(4);
    snprintf(myString,sizeof(myString), "Temp %s Sensor ERROR",tempInOut); // Show Error and exit (wbp)
    showSmTime(0,color); // Show small digit time on top
    return scrolltextsizexcolor(8,myString,RED,10);
 }
  float tempF = (tempC * 1.8) + 32.0; // Convert to Farenheit
  int tempCint = int(tempC +0.5) - tmpOffset2;// Convert round off decimal to whole number.
  int tempFint = int(tempF +0.5) - tmpOffset2; // Convert round off decimal to whole number.
  showSmTime(0,color); // Show small digit time on top
  if(!speak) { //Scroll Temp
    if (isRadioPresent) {
      if (tempUnit) snprintf(myString,sizeof(myString), "Temp %s:%dF ",tempInOut,tempFint); // Format String for Farenheight (wbp)
      else snprintf(myString,sizeof(myString), "Temp %s:%dC ",tempInOut,tempCint); // Format String for Celcius (wbp)
    }
    else {
      if (tempUnit) snprintf(myString,sizeof(myString), "Temp:%dF ",tempFint); // Format String for Farenheight (wbp)
      else snprintf(myString,sizeof(myString), "Temp:%dC ",tempCint); // Format String for Celcius (wbp)
    }
    returnVal=scrolltextsizexcolor(8,myString,color,20); 
  }
  if (speak) {
    if (tempUnit) snprintf(myString,sizeof(myString), "%dF ",tempFint); // Short Format String for Farenheight
    else  snprintf(myString,sizeof(myString), "%dC ",tempCint); // Short Format String for Farenheight
    showText(5,8,myString,1,color); // Show Static Temp string
    if (tempUnit) sayTemp(tempFint,location);
    else sayTemp(tempCint,location);
  }
  return returnVal;
}

// =======================================================================================
// ---- Show Current Date (for quick menu display) ----
// By: LensDigital
// =======================================================================================
boolean showDate(byte color){
  if (isInMenu) return true; // Do not sound alarm if changing settings
  char dateString[14]; // stores formatted date
  //showSmTime(0,color); // Show small digit time on top
  // Format Date and store in dateString array
     snprintf(dateString,sizeof(dateString), "%s %02d, %d ",monthShortStr(month()),day(),year());
     return scrolltextsizexcolor(8,dateString,color,25);
}

// ---------------------------------------------------------------------------------------




// =======================================================================================
// ---- Process Quick display function (to show Date/Temperature/Clock, etc.) ----
// ---- by LensDigital
// =======================================================================================
void quickDisplay()
{
  // ==== BEGIN Alarm Functions ====
  if ( !isInMenu && soundAlarm[0]) { // If pressed Stops Alarm (snooze)
    snoozeProc(0);
    return; //will prevent from entering menu for 1 minute since alarm sounded
   }
   if (!isInMenu && soundAlarm[1]) { // If pressed Stops Alarm (snooze)
    snoozeProc(1);
    return; //will prevent from entering menu for 1 minute since alarm sounded
   }
  // ==== END Alarm Functions ====
  
  // Read button, but only act after button is RELEASED! This will detect "HELD" or "PRESSED" state
  if (currStatusInc == HIGH) { //Button Was pressed
  // debouncing;
    if ((millis() - lastButtonTime) < BOUNCE_TIME_QUICK) return; //Debounce
    buttonReleased=false;
    buttonPressedInc=true;
    last_ms=millis(); // Set Held Timer
  }
  else { // Button was released (LOW)
    if ((millis() - lastButtonTime) < BOUNCE_TIME_QUICK) return; //Debounce
     if (buttonPressedInc) { // Checks if buttone was previously HIGH. This Eliminates bug with HOLD state
       buttonReleased=true;  
       buttonPressedInc=false; // Resets button pressed state
     }
  }
  
  if (soundAlarm[0]) interruptAlrm[0]=true; // If pressed Stops Alarm
  if (soundAlarm[1]) interruptAlrm[1]=true; // If pressed Stops Alarm
  isInQMenu=true;
  lastButtonTime = millis();
  if (buttonReleased) 
      if ( (millis() - last_ms ) > heldTime) {
        buttonReleased=false;
        talkingMenu(true); 
      }
      else {
        buttonReleased=false;
        talkingMenu(false);
      }
  
  
}

// ====================================================================================================
// ---- Talking menu (show and anounse Date/Temperature/Clock, etc.) used by quickDisplay function ----
// ---- Pass TRUE to go thru all items (i.e. button was held), or FALSE to go thru each item with press of a button
// ---- by LensDigital
// ====================================================================================================
void talkingMenu (boolean mmode) {
  isSettingAlarm=false;
  if (mbutState > 6) mbutState=1; // Go back to beginning of the menu
  if (!mmode) { //Single item talk mode
    sayItem();
    mbutState++;
  }
  else { // Multiple items talk mode (say all of them)
    mbutState=1;
    while (mbutState < 7) { // Go thru all 6 items
      sayItem();
     mbutState++;
    } // End While
    lastButtonTime = 0;// Exit QMenu
  } // End Else 
  
  //if (mmode) lastButtonTime = 0;// Exit QMenu
}


// ====================================================================================================
// ---- Talk function ----
// ---- Will announce time/date/temperature, etc. if these options enabled
// ---- by LensDigital
// ====================================================================================================
void sayItem () {
  talkingLogic ();
  switch (mbutState) {
   case 1: // Show/Say Time
      cls();
      okClock=true; 
      isSettingDate=false;
      showBigTime(clockColor);
      sayTime();
      break; 
  case 2: // Show/Say Date
      isSettingDate = true;
      okClock=false;
      cls();
      mainDate(clockColor); // Show full screen date
      sayDate();
  break; 
  case 3: // Say and show INdoor temperature
      isSettingDate = false;
      okClock=false;
      cls();
      showSmTime(0,ORANGE);
      showTemp(ORANGE,true, true); // Scroll temperature on the bottom
  break; 
  case 4: // Say and show Outdoor temperature
      isSettingDate = false;
      okClock=false;
      cls();
      showSmTime(0,ORANGE);
      showTemp(ORANGE,true, false); // Scroll temperature on the bottom
  break; 
  case 5: // Say and show Outdoor humidity
      cls();
      //showSmTime(0,ORANGE);
      showHumidity(ORANGE,true);
  break;
  case 6: // Say Alarm
      if (! (alarmon[0] & 128) && ! (alarmon[1] & 128) ) { lastButtonTime = 0; break; }//Both Alarms are off
      if ( alarmon[0] & 128) { // Alarm 1 is On
        isSettingDate = false;
        okClock=false;
        isSettingAlarm=true;
        cls();
        menuItem=1;
        subMenu[0]=4; // Enable display of Alarm 1
        showAlarm(clockColor);
        sayAlarm(0);
        
      }
      if ( alarmon[1] & 128) { // Alarm 2 is on
        menuItem=2;
        subMenu[1]=4; // Enable display of Alarm 2
        showAlarm(clockColor);
        sayAlarm(1);
     
      }
   break; 
   default: // failsafe
   lastButtonTime = 0;// Exit QMenu
   // mbutState=1;
   break;
  } 
}


// Decide which item needs to be skipped in talking menu
void talkingLogic () {
  
    switch (mbutState){ // Skip item if it's disabled in EEProm
     case 1: 
     if (!(sayOptions & 64)) {
       mbutState++;
       if (!(sayOptions & 32)) {
         mbutState++; 
         if (!(sayOptions & 16)) { 
           mbutState++;
           if (!(sayOptions & 4)) {
             mbutState++;
             if (!(sayOptions & 2)) {
               mbutState++;
               if (!(sayOptions & 8)) {
                 mbutState++;
                 return;
               }
             }
           }
         }
       }
     }
         
     break;
     case 2:
     if (!(sayOptions & 32)) {
       mbutState++; 
       if (!(sayOptions & 16)) { 
           mbutState++;
           if (!(sayOptions & 4)) {
             mbutState++;
             if (!(sayOptions & 2)) {
               mbutState++;
               if (!(sayOptions & 8)) {
                 mbutState++;
               }
             }
           }
         }
     }
     break;
     case 3:
     if (!(sayOptions & 16)) {
       mbutState++; 
       if (!(sayOptions & 4)) {
             mbutState++;
             if (!(sayOptions & 2)) {
               mbutState++;
               if (!(sayOptions & 8)) {
                 mbutState++;
               }
             }
           }
     }
     break;
     case 4:
     if (!(sayOptions & 4)) {
       mbutState++;
       if (!(sayOptions & 2)) {
               mbutState++;
         if (!(sayOptions & 8)) {
               mbutState++;
         }
       }
     }
     break;
     case 5:
       if (!(sayOptions & 2)) {
               mbutState++;
         if (!(sayOptions & 8)) {
               mbutState++;
         }
       }
     break;
     case 6:
     if (!(sayOptions & 8)) mbutState++;
     break;
    }
    
}

void startBlinking(){
  blinking = !isIncrementing;
}

void stopBlinking(){
  blinking = false;
}


// =======================================================================================
// ---- Converts 24 hours to 12 mode, returns true if AM ----
// ---- by LensDigital
// =======================================================================================
boolean hoursMode(byte hrs) {
  if (hrs == 0) myhours=12; // It's Midnight
  else if (hrs >12) myhours=hrs-12;
  if (hrs<12)
    return true; // It's AM so return true
 else 
    return false; // It's PM
}

// =======================================================================================
// ---- calc running average of last n readings
// ---- from Arduino playground (with errors fixed)
// =======================================================================================
#define LRSIZE 3
byte runningAverage(byte r)
{
  static byte LR[LRSIZE] = {2,2,2}; // previous values
  static byte index = 0;
  static long sum = 0;
//  static byte count = 0;
  sum -= LR[index];  // subtract oldest value from sum
  LR[index] = r; // replace oldest value with new
  sum += r; // add new value to sum
  index = ++index % LRSIZE; // move to new slot
//  if (count < LRSIZE) count++;
  return sum / LRSIZE; // return average
}

// =======================================================================================
// ---- Checks Ambient Light leve and adjust brightness ----
// ---- by LensDigital & William Phelps
// =======================================================================================
static unsigned long lastRun = 0;
void autoBrightness () {
  if (isInMenu) return;
  if (brightness) return; // Brightness is not set to 0 (auto)
//  if (second()%10) return; // Take readings every 10th second only
  if ((millis()-lastRun) < 500) return; // take two readings per second
  lastRun = millis();
  //Serial.println ("Changing Brightness");
  if (prevBrightness==0) {  // Initialized previous Brightness setting only if Brightness was reset
    //prevBrightness=map(analogRead(photoCellPin), PHOTOCELL_MIN, PHOTOCELL_MAX, 1, 5); // Get Ambient Light Reading
    prevBrightness=map( constrain (analogRead(photoCellPin), PHOTOCELL_MIN, PHOTOCELL_MAX), PHOTOCELL_MIN, PHOTOCELL_MAX, 1, 5); // Get Ambient Light Reading
    setBrightness(prevBrightness); // Set LED brightness
  }
//  lightLevel = map(analogRead(photoCellPin), PHOTOCELL_MIN, PHOTOCELL_MAX, 1, 5); // Get Ambient Light Reading
  lightLevel = map( constrain (analogRead(photoCellPin), PHOTOCELL_MIN, PHOTOCELL_MAX), PHOTOCELL_MIN, PHOTOCELL_MAX, 1, 5); // Get Ambient Light Reading
  lightLevel = runningAverage(lightLevel); // calc running average 
  if (lightLevel != prevBrightness) { // Set LED brightness only if light changed
    setBrightness(lightLevel);
    prevBrightness=lightLevel;
    //Serial.println (lightLevel);
    //Serial.println (FreeRam());
  }
}

// =======================================================================================
// ---- Display External Humidity  ----
// ---- speak = true if voice annoucment desired
// By: LensDigital
// =======================================================================================
boolean showHumidity(byte color, boolean speak) {
  if (!isRadioPresent) return false;
  //if (extHum==300) return false; // Humidity did not update
  char myString[25];
  showSmTime(0,color); // Show small digit time on top
  if (extHum > 100 || extHum < 1) { // Humidity sensor is not working
    //showSmTime(0,color); // Show small digit time on top
    //playSFX(4);
    return scrolltextsizexcolor(8,"Humidity Sensor ERROR",RED,10);
  }
  
  if(!speak) { //Scroll
    snprintf(myString,sizeof(myString), "Humidity %2d%%",extHum); // Scroll Outside Humidity
    return scrolltextsizexcolor(8,myString,clockColor,20);
  }
  else {
    snprintf(myString,sizeof(myString), "%d%%",extHum); // Make string for Outside Humidity  
    showText(5,8,myString,1,color); // Show Static string
    sayHumidity();
  }
  
}

// =======================================================================================
// ---- Announce External Humidity  ----
// ---- Called from showHumidity 
// By: LensDigital
// =======================================================================================
void sayHumidity() {
    char myString[8];
    //Serial.println ("Say Humidity");
    playcomplete("HUMIDITY.WAV");
    playcomplete("OUTSIDE.WAV");
    playcomplete("IS.WAV");
    if (extHum < 20) { // Say it as is
      snprintf(myString,sizeof(myString), "%d.WAV",extHum); // Scroll Outside Humidity  
      playcomplete (myString);
    }
    else { // Say 20,30, etc
     snprintf(myString,sizeof(myString), "%d0.WAV",(extHum/10)%10); // Scroll Outside Humidity  
     playcomplete (myString);
     if ((extHum%10)!=0) { // Don't say if last digit is 0
           snprintf(myString,sizeof(myString), "%d.WAV",extHum%10); // Make 2nd digit
           playcomplete(myString); // Play   
     } // end If
    } // end Else
    playcomplete ("PERCENT.WAV");
}
