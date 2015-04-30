// =======================================================================================
// ---- First time clock is powered, play startup music, show vers and say time ----
// ---------------------------------------------------------------------------------------
void startup ()
{
  if (!Settings.startup) return; // Startup Disabled?
  if (Settings.startup == 1)
    playfile("startup1.wav");
  showText(0,0,"Welcome",3,GREEN);
  showVersion();
  cls();
  showBigTime(clockColor);
}

// Show current firmware version (& other things?)
void showVersion()
{
  char welcome[15];
  byte ver=Settings.clockVer;  // 3 digit version number
  byte temp = (ver%100); //temp holder (wbp)
  byte ver3 = temp % 10; // Last digit
  byte ver2 = (temp - ver3) / 10; // Second Digit
  ver = (ver - ver2) / 100; // First digitf
  snprintf(welcome, sizeof(welcome),"Version:%d.%d%dWm",ver,ver2,ver3);  // wbp
  scrolltextsizexcolor(8,welcome,RED,15);
  cls();
}

// Display divider colon :
void showDivider(byte color){
  int x=0; //offset
//  if(time12hr) x=1; // Offset divider by 2 dots left for 12 hr mode
//  if (clockFont<3) {
    plot (15-x,5,color);
    plot (15-x,6,color);
    plot (15-x,10,color);
    plot (15-x,11,color);
//  }
  plot (16-x,5,color);
  plot (16-x,6,color);
  plot (16-x,10,color);
  plot (16-x,11,color);
}

// =======================================================================================
// ---- Info display ----
// By: LensDigital
// ---------------------------------------------------------------------------------------
void infoDisplay() {
  if (Settings.infoFreq == 0) return; // User choose to never show this screen
  if (! (Settings.infoOptions & (IO_Date+IO_Alarms+IO_InTemp+IO_InHum+IO_OutTemp+IO_OutHum) ) ) return; // All display options disabled
  if ((alarmState[0]==AS_SOUNDING) || (alarmState[1]==AS_SOUNDING)) return; // Do not show if Alarm is playing
  if (isInMenu) return; // Do not show when in menu
  if (isInQMenu) return; // Do not show when in quick menu
  // Check if it's time to Show Info display 
  if ( (minute()%10)%Settings.infoFreq == 0 && ( second()==5) )  { // Show date and temp every XX minutes and 5 seconds
    char myString[44];
    //if ( (minute()%10)==0) return; // Prevents showing during 0 digit time
    cls();
    showSmTime(0,clockColor); // Show Time on top
    if (Settings.infoOptions & IO_Date) // Option Enabled?
      if (!showDate(clockColor)) return; // Scroll Date. Exit if scroll was interrupted by button press
    if (Settings.infoOptions & IO_InTemp) { // Option Enabled?
      if (!showTemp(clockColor,false,true)) return; // Scroll Inside Temp. Exit if scroll was interrupted by button press
      cls();
    }
    if (Settings.infoOptions & IO_InHum) { // Option Enabled?
      if (!showHumidity(clockColor,false,true)) return; // Scroll Inside Humidity. Exit if scroll was interrupted by button press
      cls();
    }
    if (Settings.RadioEnabled) {
        if (Settings.infoOptions & IO_OutTemp) { // Option Enabled?
          if (!showTemp(clockColor,false,false)) return; // Scroll Outside Temp. Exit if scroll was interrupted by button press
          cls();
        }
        if (Settings.infoOptions & IO_OutHum) { // Humidity Option Enabled?
          if (!showHumidity(clockColor,false,false)) return; //Scroll Outside Humidity. Exit if scroll was interrupted by button press
          cls ();
        }
    }
    if (Settings.infoOptions & IO_Alarms) {// Option Enabled?
      if (!alarmInfo(0)) return; //Show info for alarm 1. Exit if scroll was interrupted by button press
      cls ();
      if (!alarmInfo(1)) return; //Show info for alarm 2. Exit if scroll was interrupted by button press
      cls();
    }
    if (Settings.RadioEnabled) 
      if (Settings.infoOptions & IO_Sensor) {// Option Enabled?
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
  if ( ! (Settings.alarmOn[alrmNum] & 128) ) return true; //  Alarm is off
     char myString[54]; // String to keep Alarm msg
     char wkdays[28]; //Days of week
     showSmTime(0,clockColor); // Show time on top
     if ( alarmState[alrmNum] == AS_SNOOZING ) { // Are we snoozing?
       snprintf(myString,sizeof(myString), "Alarm%d on! Snoozing...Z Z z z z z...",alrmNum+1);
       return scrolltextsizexcolor(8,myString,RED,5);
     }
     else { // Alarm is on but not snoozing
       if (Settings.alarmOn[alrmNum] == DAILY) {  // Daily Alarm
         if(Settings.time12hr) { // It's 12 hour mode
           if (hoursMode(Settings.alarmHH[alrmNum]) )  // Check if it's AM
             snprintf(myString,sizeof(myString), "Alarm %d is set Daily at %d:%02d AM",alrmNum+1,myhours,Settings.alarmMM[alrmNum]); 
           else // It's PM
             snprintf(myString,sizeof(myString), "Alarm %d is set Daily at %d:%02d PM",alrmNum+1,myhours,Settings.alarmMM[alrmNum]); 
         }
         else // It's 24 hour mode
           snprintf(myString,sizeof(myString), "Alarm %d is set Daily at %02d:%02d",alrmNum+1,Settings.alarmHH[alrmNum],Settings.alarmMM[alrmNum]); 
       }
       else if (Settings.alarmOn[alrmNum] == WEEKDAY) {  // It's WeekDay alarm
         if(Settings.time12hr) { // It's 12 hour mode
           myhours = Settings.alarmHH[alrmNum];
           if (myhours == 0) 
             myhours=12; // It's Midnight
           else if (myhours >12) myhours=myhours-12;
           if (Settings.alarmHH[alrmNum]<12) // Show AM
             snprintf(myString,sizeof(myString), "Alarm %d is set M-F at %d:%02d AM",alrmNum+1,myhours,Settings.alarmMM[alrmNum]); 
           else 
             snprintf(myString,sizeof(myString), "Alarm %d is set M-F at %d:%02d PM",alrmNum+1,myhours,Settings.alarmMM[alrmNum]); 
         }
         else // It's 24 hour mode
         snprintf(myString,sizeof(myString), "Alarm %d is set M-F at %02d:%02d",alrmNum+1,Settings.alarmHH[alrmNum],Settings.alarmMM[alrmNum]); 
       }
       else {  // It's Custom alarm
          // Make custom string containing each weekday that alarm is set too
         strcpy (wkdays,"");
         if (Settings.alarmOn[alrmNum] & MON)  strcat (wkdays, "Mon ");
         if (Settings.alarmOn[alrmNum] & TUE)  strcat (wkdays, "Tue ");
         if (Settings.alarmOn[alrmNum] & WED)  strcat (wkdays, "Wed ");
         if (Settings.alarmOn[alrmNum] & THU)  strcat (wkdays, "Thu ");
         if (Settings.alarmOn[alrmNum] & FRI)  strcat (wkdays, "Fri ");
         if (Settings.alarmOn[alrmNum] & SAT)  strcat (wkdays, "Sat ");
         if (Settings.alarmOn[alrmNum] & SUN)  strcat (wkdays, "Sun ");
           
          if(Settings.time12hr) { // It's 12 hour mode
           myhours = Settings.alarmHH[alrmNum];
           if (myhours == 0) 
             myhours=12; // It's Midnight
           else if (myhours >12) myhours=myhours-12;
           if (Settings.alarmHH[alrmNum]<12) // Show AM
             snprintf(myString,sizeof(myString), "Alarm%d is set to %d:%02d AM %s",alrmNum+1,myhours,Settings.alarmMM[alrmNum],wkdays); 
           else 
             snprintf(myString,sizeof(myString), "Alarm%d is set to %d:%02d PM %s",alrmNum+1,myhours,Settings.alarmMM[alrmNum],wkdays); 
         }
         else // It's 24 hour mode
         snprintf(myString,sizeof(myString), "Alarm%d is set to %02d:%02d %s",alrmNum+1,Settings.alarmHH[alrmNum],Settings.alarmMM[alrmNum],wkdays); 
       }
     return scrolltextsizexcolor(8,myString,clockColor,5);
     }
     
}


// =======================================================================================
// ---- Says current time ----
// By: LensDigital
// ---------------------------------------------------------------------------------------
void sayTime(){
  unsigned long tNow = now();  // time now 
     playcomplete("TIME_IS.WAV");
     char myString[9];
     myhours = hour(tNow);
     if(Settings.time12hr) { // == 12 Hour Mode ====
       if (myhours>12)  myhours-=12;
       if (myhours==0)  myhours=12;
       snprintf(myString,sizeof(myString), "%d.WAV",myhours); // Make Hours string
       playcomplete(myString); // Play Hours
     }
     else // == 24 Hour Mode ====
       if (hour(tNow) > 20) { // Make complex 2 digit sound
         playcomplete("20.WAV"); // Play Hours 1st digit
         snprintf(myString,sizeof(myString), "%d.WAV",hour(tNow)%10); // Make Hours 2nd digit string
         playcomplete(myString); // Play Hours 2nd digit
       }
       else { // Simple
         snprintf(myString,sizeof(myString), "%d.WAV",hour(tNow)); // Make Hours string
         playcomplete(myString); // Play Hours
       }
     if (minute(tNow)<20) {
       if (minute(tNow)==0) {
         if (!Settings.time12hr)  // only say "hundred" if in 24 hour mode (wbp)
           playcomplete("100.WAV");  // We are at hh:00
       }
       else {
         if (( (minute(tNow)/10)%10) == 0) playcomplete ("OH.WAV"); // If first digit of minute is 0, say "oh"
         snprintf(myString,sizeof(myString), "%d.WAV",minute(tNow)); // Make Minutes string
         playcomplete(myString);
         }
       }
     else { // Make complex 2 digit sound
       minutes=(minute(tNow)/10)%10;
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
       if ((minute(tNow)%10)!=0) { // Don't say if last digit is 0
         snprintf(myString,sizeof(myString), "%d.WAV",minute(tNow)%10); // Make 2nd digit
         playcomplete(myString); // Play 
       }
     }
     // If needed say AM or PM
     if(Settings.time12hr) { // == 12 Hour Mode ====
       if (isAM(tNow) ) playcomplete("AM.WAV");
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
  
  if(Settings.time12hr) { // It's 12 hour mode
           myhours = Settings.alarmHH[alrmNum];
           if (myhours == 0) 
             myhours=12; // It's Midnight
           else if (myhours >12) myhours=myhours-12;
           snprintf(myString,sizeof(myString), "%d.WAV",myhours); // Make hours filename
           playcomplete(myString); // Play Hours
   }
     else // == 24 Hour Mode ====
       if (Settings.alarmHH[alrmNum] > 20) { // Make complex 2 digit sound
         playcomplete("20.WAV"); // Play Hours 1st digit
         snprintf(myString,sizeof(myString), "%d.WAV",Settings.alarmHH[alrmNum]%10); // Make Hours 2nd digit string
         playcomplete(myString); // Play Hours 2nd digit
       }
       else { // Simple
         snprintf(myString,sizeof(myString), "%d.WAV",Settings.alarmHH[alrmNum]); // Make Hours string
         playcomplete(myString); // Play Hours
       }
     if (Settings.alarmMM[alrmNum]<20) {
       if (Settings.alarmMM[alrmNum]==0) { playcomplete("100.WAV"); } // We are at hh:00
       else {
         if (( (Settings.alarmMM[alrmNum]/10)%10) == 0) playcomplete ("OH.WAV"); // If first digit of minute is 0, say "oh"
         snprintf(myString,sizeof(myString), "%d.WAV",Settings.alarmMM[alrmNum]); // Make Minutes string
         playcomplete(myString);
         }
       }
     else { // Make complex 2 digit sound
       minutes=(Settings.alarmMM[alrmNum]/10)%10;
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
       if ((Settings.alarmMM[alrmNum]%10)!=0) { // Don't say if last digit is 0
         snprintf(myString,sizeof(myString), "%d.WAV",Settings.alarmMM[alrmNum]%10); // Make 2nd digit
         playcomplete(myString); // Play 
       }
     }
     // If needed say AM or PM
     if(Settings.time12hr) { // == 12 Hour Mode ====
       if (Settings.alarmHH[alrmNum]<12)  playcomplete("AM.WAV");
       else playcomplete("PM.WAV");
     }
     if (Settings.alarmOn[alrmNum] == DAILY) playcomplete("DAILY.WAV");// Daily Alarm
     else if (Settings.alarmOn[alrmNum] == WEEKDAY) playcomplete("wkdays.WAV");// Workday Alarm
     else { // Custom ALarm
       playcomplete("EVERY.WAV");
       if (Settings.alarmOn[alrmNum] & MON) playcomplete("MON.WAV");
       if (Settings.alarmOn[alrmNum] & TUE) playcomplete("TUE.WAV");
       if (Settings.alarmOn[alrmNum] & WED) playcomplete("WED.WAV");
       if (Settings.alarmOn[alrmNum] & THU) playcomplete("THU.WAV");
       if (Settings.alarmOn[alrmNum] & FRI) playcomplete("FRI.WAV");
       if (Settings.alarmOn[alrmNum] & SAT) playcomplete("SAT.WAV");
       if (Settings.alarmOn[alrmNum] & SUN) playcomplete("SUN.WAV");
     }
     
}

// =======================================================================================
// ---- Announce Inside Temperature ----
// By: LensDigital
// ---------------------------------------------------------------------------------------
void sayTemp(int temp, boolean inside){
  /*
  if (temp > 100 || temp < -50) {
    playcomplete ("ERR1.WAV");
    return;
  } */
  char myString[8];
  playcomplete("TEMP.WAV");
  if (Settings.RadioEnabled) {  // if no radio, no need to qualify which temperature it is
    if (inside) playcomplete("INSIDE.WAV");
    else playcomplete("OUTSIDE.WAV");
  }
  playcomplete("IS.WAV");
  if (temp<20) {
    snprintf(myString,sizeof(myString), "%d.WAV",temp); // Teen and single digits
    playcomplete(myString);
  }
  else { // It's over 20, so make 2 digit phrase
    snprintf(myString,sizeof(myString), "%d0.WAV",(temp/10)%10); // first digit
    playcomplete(myString);
    if ((temp%10)!=0) {
      snprintf(myString,sizeof(myString), "%d.WAV",temp%10); // first digit
      playcomplete(myString);
    }
  }
  playcomplete("DEGREES.WAV");
  if (Settings.tempUnit) playcomplete("fahrenh.WAV");
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
//  Serial.println ("Say Date!");
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
// set common blink switches
// By: wbp
// ---------------------------------------------------------------------------------------
#define blinkDotDuration 1000 // How frequently dots should blink (wbp)
#define blinkDigitDuration 500 // Frequencey of digit blinking during time setting
void setBlinkers(){
  // Blinker processor (used to blink divider and/or digits during time setting
  if ( (millis()-blinkTime > blinkDotDuration)) { // It's been over blinkDuration time
    blinkTime = millis(); // reset blink timer to current time
    blinkOn = !blinkOn; // toggle blinkOn
  }
  if ( (millis()-blinkDigitTime > blinkDigitDuration)) { // It's been over blinkDuration time
    blinkDigitTime = millis(); // reset blink timer to current time
    blinkDigit = !blinkDigit; // toggle blinkOn
  }
}


// =======================================================================================
// Display large digit (full screen) time
// By: LensDigital
// modified by wbp
// ---------------------------------------------------------------------------------------
void showBigTime(byte color){
  //if (menuItem > 3 || !okClock) return; // Date setting is in progress. Do not show clock
  if (!okClock) return; // Date setting is in progress. Do not show clock
  if (color==4)  color=random(3)+1; // Select random color
  
  // is divider (colon) blinking?
  if (blinkOn || isInMenu || !Settings.cursorBlink) blinkColor = color;
  else blinkColor = BLACK;  // blink the divider

  hhColor = color;  mmColor = color;  // assume normal color
  if (blinking) { // Setting time, so blink correct digits
    //Serial.println ("Setting time");
    blinkColor=color; // Show steady divider dots
      if (blinkDigit) {  // blink?
        if (isSettingHours)  hhColor = BLACK;  // blinking Hours
        if (isSettingMinutes) mmColor = BLACK;  // blinking Minutes
      }
  }
  // --- END OF BLINK PROCESSOR
    
  unsigned long tNow = now();  // wbp
  myhours = hour(tNow);
  // Check if we are running in 12 Hour Mode:
  if(Settings.time12hr) {
  // == BEGIN 12 Hour Mode ====
//    x=2; //offset hours by 2 dots
//    myhours=hourFormat12();
//    if (isAM()) plot (0,1,hhColor); // Show AM Dot
    if (isPM(tNow)) plot (0,1,hhColor); // Show Dot for PM
    else plot (0,1,BLACK); // Hide Dot
    if (myhours>12)  myhours-=12;
    if (myhours==0)  myhours=12;  
    if ( (myhours/10)%10 == 0 ) showDigit(-1,2,0,5,Settings.clockFont,BLACK); // Hide first digit 
    else showDigit(-1,2,(myhours/10)%10,5,Settings.clockFont,hhColor);
  }
  // === END 12 Hour Mode ===
  else {
  // 24 Hour Mode
    //plot (0,1,BLACK); // Hide PM Dot (wbp)
    showDigit(-1,2,(myhours/10)%10,5,Settings.clockFont,hhColor); // Show 1st digit of hour
  }
  showDigit(5,2,myhours%10,5,Settings.clockFont,hhColor); // Show 2nd digit of hour
  showDivider (blinkColor);
  showDigit(16,2,(minute(tNow)/10)%10,5,Settings.clockFont,mmColor); // Show 1st digit of minute
  showDigit(22,2,minute(tNow)%10,5,Settings.clockFont,mmColor); // Show 2nd digit of minute
  
}


// =======================================================================================
// ---- Display smaller digit time ----
// By: LensDigital
// Location is either 0: top row, or 1: buttom row
// ---------------------------------------------------------------------------------------
void showSmTime (byte location,byte color){
  char myString[6];
  unsigned long tNow = now();
  location=location*8; // Shift to bottom row if 1
  // Check if we are running in 12 Hour Mode:
  myhours = hour(tNow);
  if (Settings.time12hr) {
  // == BEGIN 12 Hour Mode ====
//    myhours=hourFormat12();
    if (myhours>12)  myhours-=12;  // 12 hour time
    if (myhours==0)  myhours=12;  // midnight
//    if (isAM()) plot (0,1,color); // Show AM Dot
    if (isPM(tNow)) plot (0,1,color); // Show Dot for PM
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
   snprintf(myString,sizeof(myString), "%02d",myhours); // make 2 digit hours
   showText(1,location,myString,1,color);
    //plot (0,1,BLACK); // Hide PM Dot
  }
  showText(12,location,":",1,color); // Show colon :
  snprintf(myString,sizeof(myString), "%02d",minute(tNow));
  showText(18,location,myString,1,color); // Show minutes
}

// =======================================================================================
// ---- Display Date and year (mostly for setting) ----
// By: LensDigital
// ---------------------------------------------------------------------------------------
void mainDate(byte color){
//  if (!isSettingDate) return; // Not setting Date
  char dateString[5]; // Var will hold generated string
  if (color==4)color=random(3)+1; // Select random color
  // Blink processor (used to blink text, number during date setting
  monColor=color;
  ddColor=color;
  yyColor=color;
  if (isSettingTime && blinking && blinkDigit){
    if (isSettingMonth) {
       monColor=BLACK; 
     }
     if (isSettingDay) {
       ddColor=BLACK;
     }
     if (isSettingYear) {
       yyColor=BLACK;
     }
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
boolean showTemp(byte color, boolean speak, boolean inside){
  char myString[32];
  char tempInOut[7];
  byte tmpOffset2=0; // will differ if getting temp from outside
  if (inside)  tmpOffset2=Settings.tempOffset; //Only for Inside temp Actual offset is used
  boolean returnVal=true;// Return value
  float tempC;
  if (inside) { // Get temperature from attached sensor
#if defined DS18B20
    sensors.requestTemperatures(); // Send the command to get temperatures
    tempC = sensors.getTempC(insideThermometer);
#elif defined DHT22
    tempC = dht.readTemperature();
#endif
    snprintf(tempInOut,sizeof(tempInOut), "In");
  }
  else { // Get Temperature from external sensor
    if (!Settings.RadioEnabled)  return true;
    tempC=extTemp; // External Temperature was requested
    snprintf(tempInOut,sizeof(tempInOut), "Out");
  }
  if (tempC > 100 || tempC < -50) { // Temperature sensor is not working
    //playSFX(5);
    snprintf(myString,sizeof(myString), "Temp %s Sensor ERROR",tempInOut); // Show Error and exit (wbp)
    showSmTime(0,color); // Show small digit time on top
    return scrolltextsizexcolor(8,myString,RED,10);
 }
  float tempF = (tempC * 1.8) + 32.0; // Convert to Farenheit
  int tempCint = int(tempC +0.5) - tmpOffset2;// Convert round off decimal to whole number.
  int tempFint = int(tempF +0.5) - tmpOffset2; // Convert round off decimal to whole number.
  showSmTime(0,color); // Show small digit time on top
  if(!speak) { //Scroll Temp
    if (Settings.RadioEnabled) {
      if (Settings.tempUnit) snprintf(myString,sizeof(myString), "Temp %s:%dF ",tempInOut,tempFint); // Format String for Farenheight (wbp)
      else snprintf(myString,sizeof(myString), "Temp %s:%dC ",tempInOut,tempCint); // Format String for Celcius (wbp)
    }
    else {
      if (Settings.tempUnit) snprintf(myString,sizeof(myString), "Temp:%dF ",tempFint); // Format String for Farenheight (wbp)
      else snprintf(myString,sizeof(myString), "Temp:%dC ",tempCint); // Format String for Celcius (wbp)
    }
    returnVal=scrolltextsizexcolor(8,myString,color,20); 
  }
  else {  // speak
    if (Settings.tempUnit) snprintf(myString,sizeof(myString), "%dF ",tempFint); // Short Format String for Farenheight
    else  snprintf(myString,sizeof(myString), "%dC ",tempCint); // Short Format String for Farenheight
    showText(5,8,myString,1,color); // Show Static Temp string
    if (Settings.tempUnit) sayTemp(tempFint,inside);
    else sayTemp(tempCint,inside);
  }
  return returnVal;
}

// =======================================================================================
// ---- Show Current Date (for quick menu display) ----
// By: LensDigital
// =======================================================================================
boolean showDate(byte color){
  if (isInMenu) return true; // Do not display date if changing settings
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
void quickDisplay(boolean doAll)
{
  // ==== BEGIN Alarm Functions ====
  if ( !isInMenu && (alarmState[0] == AS_SOUNDING)) { // If pressed Stops Alarm (snooze)
    snoozeProc(0);
    return; //will prevent from entering menu for 1 minute since alarm sounded
   }
   if (!isInMenu && (alarmState[1] == AS_SOUNDING)) { // If pressed Stops Alarm (snooze)
    snoozeProc(1);
    return; //will prevent from entering menu for 1 minute since alarm sounded
   }
  // ==== END Alarm Functions ====
  
//  if (soundAlarm[0]) interruptAlrm[0]=true; // If pressed Stops Alarm
//  if (soundAlarm[1]) interruptAlrm[1]=true; // If pressed Stops Alarm
  isInQMenu=true;
//  lastButtonTime = millis();
  talkingMenu(doAll);
}

// ====================================================================================================
// ---- Talking menu (show and announce Date/Temperature/Clock, etc.) used by quickDisplay function ----
// ---- Pass TRUE to go thru all items (i.e. button was held), or FALSE to go thru each item with press of a button
// ---- by LensDigital
// ====================================================================================================
void talkingMenu (boolean mmode) {
  isSettingAlarm=false;
  if (talkingItem > 7) talkingItem=1; // Go back to beginning of the menu
  if (!mmode) { //Single item talk mode
    sayItem(talkingItem);
    talkingItem = talkingLogic(++talkingItem);
  }
  else { // Multiple items talk mode (say all of them)
    talkingItem=1;
    while (talkingItem < 8) { // Go thru all 7 items
      sayItem(talkingItem);
      talkingItem = talkingLogic(++talkingItem);
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
void sayItem (byte item) {
//  item = talkingLogic (item);
  switch (item) {
   case 1: // Show/Say Time
      cls();
      okClock=true; 
//      isSettingDate=false;
      showBigTime(clockColor);
      sayTime();
      break; 
  case 2: // Show/Say Date
//      isSettingDate = true;
      okClock=false;
      cls();
      mainDate(clockColor); // Show full screen date
      sayDate();
  break; 
  case 3: // Say and show Indoor temperature
//      isSettingDate = false;
      okClock=false;
      cls();
      showSmTime(0,ORANGE);
      showTemp(ORANGE,true, true); // Scroll temperature on the bottom
  break; 
  case 4: // Say and show Indoor humidity
//      isSettingDate = false;
      okClock=false;
      cls();
      showSmTime(0,ORANGE);
      showHumidity(ORANGE,true, true); // Scroll humidity on the bottom
  break; 
  case 5: // Say and show Outdoor temperature
//      isSettingDate = false;
      okClock=false;
      cls();
      showSmTime(0,ORANGE);
      showTemp(ORANGE,true, false); // Scroll temperature on the bottom
  break; 
  case 6: // Say and show Outdoor humidity
      cls();
      //showSmTime(0,ORANGE);
      showHumidity(ORANGE,true,false);
  break;
  case 7: // Say Alarm
      if (! (Settings.alarmOn[0] & 128) && ! (Settings.alarmOn[1] & 128) ) { lastButtonTime = 0; break; }//Both Alarms are off
      if ( Settings.alarmOn[0] & 128) { // Alarm 1 is On
//        isSettingDate = false;
        okClock=false;
//        isSettingAlarm=true;  wbp - not setting the alarm now
        cls();
        menuItem=1;
        subMenu[0]=4; // Enable display of Alarm 1
        showAlarm(clockColor);
        sayAlarm(0);
      }
      // wbp: what happens if alarm 2 is on but not alarm 1? no cls, etc ???
      if ( Settings.alarmOn[1] & 128) { // Alarm 2 is on
        menuItem=2;
        subMenu[1]=4; // Enable display of Alarm 2
        showAlarm(clockColor);
        sayAlarm(1);
      }
   break; 
   default: // failsafe
   lastButtonTime = 0;// Exit QMenu
   // talkingItem=1;
   break;
  } 
}


// Decide which item needs to be skipped in talking menu
byte talkingLogic (byte talkingItem) {
  switch (talkingItem) { // Skip item if it's disabled in EEProm
    case 1:  // are we speaking the Time?
      if (Settings.sayOptions & SO_Time)  break;
      talkingItem++;  // skip this one
    case 2:  // Date?
      if (Settings.sayOptions & SO_Date)  break;
      talkingItem++;  // skip this one
    case 3:  // Inside Temp?
      if (Settings.sayOptions & SO_InTemp)  break;
      talkingItem++;  // skip this one
    case 4:  // Inside Humidity
      if (Settings.sayOptions & SO_InHum)  break;
      talkingItem++;  // skip this one
    case 5:  // Outside Temp
      if (Settings.sayOptions & SO_OutTemp)  break;
      talkingItem++;  // skip this one
    case 6:  // Outside Humidity
      if (Settings.sayOptions & SO_OutHum)  break;
      talkingItem++;  // skip this one
    case 7:  // Alarms?
      if (Settings.sayOptions & SO_Alarms)  break;
      talkingItem++;  // skip this one
  }
  return talkingItem;
}

void startBlinking(){
  blinking = !isAdjusting;
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
// =======================================================================================
#define LRSIZE 3  // average of 3 readings
static int LR[LRSIZE] = {2,2,2}; // previous values
static byte LRindex = 0;
byte runningAverage(int r)
{
  int sum = 0;
  LR[LRindex] = r; // replace oldest value with new
  if (++LRindex == LRSIZE) // increment index 
    LRindex = 0; // and wrap
  for (byte i = 0; i<LRSIZE; i++) {
    sum += LR[i];
  }
  return sum / LRSIZE; // return average
}

// =======================================================================================
// ---- Checks Ambient Light level and adjust brightness ----
// ---- by LensDigital & William Phelps
// =======================================================================================
static unsigned long lastRun = 0;
void autoBrightness () {
//  if (isInMenu) return;
  if (Settings.brightness) return; // Brightness is not set to 0 (auto)
//  if (second()%10) return; // Take readings every 10th second only
  if ((millis()-lastRun) < 1000) return; // take one reading per second
  lastRun = millis();
  //Serial.println ("Changing Brightness");
  photoCell = analogRead(photoCellPin);
//  Serial.print("pCell:"); Serial.println(photoCell);
  lightLevel = map( constrain (photoCell, Settings.photoCellMin, Settings.photoCellMax), Settings.photoCellMin, Settings.photoCellMax, 1, 5); // Get Ambient Light Reading
  lightLevel = runningAverage(lightLevel); // calc running average 
//  if (prevBrightness==0) {  // Initialized previous Brightness setting only if Brightness was reset
//    prevBrightness=lightLevel;
//  }
  if (lightLevel != prevBrightness) { // Set LED brightness only if light changed
    setBrightness(lightLevel);
    prevBrightness=lightLevel;
    //Serial.println (lightLevel);
    //Serial.println (FreeRam());
    if (Settings.autoColor) {
      if (lightLevel < 3)
        clockColor = RED;
      else if (lightLevel < 4)
        clockColor = ORANGE;
      else
        clockColor = GREEN;
    }
  }
}

// =======================================================================================
// ---- Display External Humidity  ----
// ---- speak = true if voice annoucment desired
// By: LensDigital
// =======================================================================================
boolean showHumidity(byte color, boolean speak, boolean inside) {
  char myString[25];
  int humid;
  if (inside) { // Get humidity from attached sensor
#if defined DS18B20
    return false;  // no humidity sensor
#elif defined DHT22
    showSmTime(0,color); // Show small digit time on top
    humid = dht.readHumidity();
#endif
  }
  else { // Get Temperature from external sensor
    if (!Settings.RadioEnabled) return false;
    //if (extHum==300) return false; // Humidity did not update
    showSmTime(0,color); // Show small digit time on top
    if (extHum > 100 || extHum < 1) { // Humidity sensor is not working
      //showSmTime(0,color); // Show small digit time on top
      //playSFX(5);
      return scrolltextsizexcolor(8,"Humidity Sensor ERROR",RED,10);
    }
    humid = extHum;
  }
  if(!speak) { //Scroll
    snprintf(myString,sizeof(myString), "Humid %2d%%",humid); // Scroll Humidity
    return scrolltextsizexcolor(8,myString,clockColor,20);
  }
  else {
    snprintf(myString,sizeof(myString), "%d%%",humid); // Make string for Humidity
    showText(5,8,myString,1,color); // Show Static string
    sayHumidity(humid, inside);
  }
}

// =======================================================================================
// ---- Announce External Humidity  ----
// ---- Called from showHumidity 
// By: LensDigital
// =======================================================================================
void sayHumidity(int hum, byte inside) {
  char myString[8];
  //Serial.println ("Say Humidity");
  playcomplete("HUMIDITY.WAV");
  if (Settings.RadioEnabled) {  // if no radio, no need to qualify which temperature it is
    if (inside)
      playcomplete("INSIDE.WAV");
    else
      playcomplete("OUTSIDE.WAV");
  }
  playcomplete("IS.WAV");
  if (hum < 20) { // Say it as is
    snprintf(myString,sizeof(myString), "%d.WAV",hum); // Scroll Outside Humidity  
    playcomplete (myString);
  }
  else { // Say 20,30, etc
    snprintf(myString,sizeof(myString), "%d0.WAV",(hum/10)%10); // Scroll Outside Humidity  
    playcomplete (myString);
    if ((hum%10)!=0) { // Don't say if last digit is 0
      snprintf(myString,sizeof(myString), "%d.WAV",hum%10); // Make 2nd digit
      playcomplete(myString); // Play   
    } // end If
  } // end Else
  playcomplete ("PERCENT.WAV");
}
