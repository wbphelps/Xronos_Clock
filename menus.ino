// =======================================================================================
// ---- Show Menu items and set vars ----
// ---- by LensDigital
// =======================================================================================
void showMenu() 
{
 playSFX(2);
//  for (int i=0;i<MAX_SUBMENUS+1;i++) { // Initialize SubMenus
  for (int i=0;i<MAX_SUBMENUS;i++) { // Initialize SubMenus  (wbp)
      subMenu[i]=0;
   }
     
  switch (menuItem) {
    case 1: // Set Alarm 1 Menu Item (Display Only)
      putstring_nl ("Display Alrm 1 Menu");
      okClock=false;
      isSettingSys=false;
      isSettingOptions=false;
      cls();
      showText(1,0,"Set",1,ORANGE);
      showText(3,8,"Alrm1",1,ORANGE);
      break;
    case 2: // Set Alarm 2 Menu Item (Display Only)
      putstring_nl ("Display Alrm 2 Menu");
      okClock=false;
      isSettingSys=false;
      cls();
      showText(1,0,"Set",1,ORANGE);
      showText(3,8,"Alrm2",1,ORANGE);
      break;
    case 3: // Time and Date setting menu
      putstring_nl ("Display Date/Time Menu");
      okClock=false;
      isSettingSys=false;
      cls();
      showText(1,0,"Time/",1,ORANGE);
      showText(5,8,"Date",1,ORANGE);
      break;
   case 4: // System Settings (preferences/setup)
      putstring_nl ("Display System Menu");
      okClock=false;
      cls();
      showText(1,0,"Sys",1,ORANGE);
      showText(3,8,"Setup",1,ORANGE);
      break;
   case 5: // User Options menu
      putstring_nl ("Display Options Menu");
      okClock=false;
      isSettingSys=false;
      cls();
      showText(1,0,"User",1,ORANGE);
      showText(3,8,"Menu",1,ORANGE);
      break;
  } 
}


// =======================================================================================
// ---- Set Alarm ----
// ---- by LensDigital
// =======================================================================================
void setAlarm(byte alrmNum) {
  char myString[11];
  interruptAlrm[alrmNum]=false; //Rearm Button interrupt
  soundAlarm[alrmNum]=false; // Make sure alarm that just sounded doesn't resume
  snoozeTime[alrmNum]=10; // Disable Snooze
//  if (alrmToneNum[alrmNum]<=ALARM_PROGRESSIVE) alrmVol[alrmNum]=7; // Set initial alarm volume (for escalating alarms)
  if (alrmProgVol[alrmNum]) alrmVol[alrmNum]=7; // Set initial alarm volume (for escalating alarms)
  if (subMenu[alrmNum] >0 && subMenu[alrmNum] <4) playSFX(1); // Don't play sound if not setting anything, i.e. submenu=0 or setting hrs, minutes, tone
  switch (subMenu[alrmNum]) {
    case 1: // Alarm On/off
      alarmon[alrmNum]=alarmon[alrmNum] ^ 128; //Toggle first bit (on/off)
      delay (5);
      EEPROM.write (alarmOnOffLoc[alrmNum],alarmon[alrmNum]);
      cls();
      break;
    case 2: // Set Alarm frequency to off/daily/weekday/custom
      switch (alarmon[alrmNum]){
        case 255:
          alarmon[alrmNum]=252; // Was Daily, now Workday
          isSettingAlrmCust[alrmNum] =false;
          break; 
        case 252:
          alarmon[alrmNum]=254; // Was Workday, now custom
          isSettingAlrmCust[alrmNum] =true;
          break;
        default:
          alarmon[alrmNum]=255; // Was custom, now Daily
          isSettingAlrmCust[alrmNum] =false; // Remove if want to keep custom Menu setting even if alarm is off (need to tweak to make cusotm right after OFF so it will remember day settings 
        }
      EEPROM.write (alarmOnOffLoc[alrmNum],alarmon[alrmNum]);
      delay (5);
      playSFX(1);
      cls();
      break;
    case 3: // Custom Day selection
      // Check if Custom Seeting was picked, if not skip this option
      switch (subMenu[4+alrmNum]) {
        case 1: // Mon
          alarmon[alrmNum]=alarmon[alrmNum] ^ 64; //Toggle
          break;
        case 2: // Tue
          alarmon[alrmNum]=alarmon[alrmNum] ^ 32; //Toggle 
          break;
        case 3: // Wed
          alarmon[alrmNum]=alarmon[alrmNum] ^ 16; //Toggle 
          break;
        case 4: // Thu
          alarmon[alrmNum]=alarmon[alrmNum] ^ 8; //Toggle 
          break;
        case 5: // Fri
          alarmon[alrmNum]=alarmon[alrmNum] ^ 4; //Toggle 
          break;
        case 6: // Sat
          alarmon[alrmNum]=alarmon[alrmNum] ^ 2; //Toggle 
          break;
        case 7: // Sun
          alarmon[alrmNum]=alarmon[alrmNum] ^ 1; //Toggle 
          break;
        case 8: // Exit
          subMenu[4+alrmNum]=0;
          subMenu[alrmNum]=4;//Move to next menu item
          cls();
          break;
      }
      EEPROM.write (alarmOnOffLoc[alrmNum],alarmon[alrmNum]);
      playSFX(1);
      cls();
      break;
    case 4: // Set Alarm hours
      isAlarmModified[alrmNum]=true; // Set to True so when exiting changes will be written to EEProm
      if (decrement) alrmHH[alrmNum]--; 
      else alrmHH[alrmNum]++;
      if (alrmHH[alrmNum]==255) alrmHH[alrmNum] = 23; // Negative number (byte) will be 255
      else if (alrmHH[alrmNum] > 23) alrmHH[alrmNum] = 0;      
      break;
    case 5: // Set Alarm minutes  
      isAlarmModified[alrmNum]=true; // Set to True so when exiting changes will be written to EEProm
      if (decrement) alrmMM[alrmNum]--; 
      else alrmMM[alrmNum]++;
      if (alrmMM[alrmNum] ==255) alrmMM[alrmNum] = 59; // Negative number (byte) will be 255
      else if (alrmMM[alrmNum] > 59) alrmMM[alrmNum] = 0;
      break;
    case 6: // Set Alarm melody  
      if (decrement) alrmToneNum[alrmNum]--;
      else alrmToneNum[alrmNum]++;
      wave.stop();
      if (alrmToneNum[alrmNum] == 255) alrmToneNum[alrmNum] = 10; // Negative number (byte) will be 255
      else if (alrmToneNum[alrmNum] > 10) alrmToneNum[alrmNum] = 1;
      snprintf(myString,sizeof(myString), "ALRM%d.WAV",alrmToneNum[alrmNum]); // Make Alarm Filename
      playfile(myString);
      EEPROM.write(alarmToneLoc[alrmNum],alrmToneNum[alrmNum]);
      delay (5);
      cls();
      break;
    case 7: // Alarm Progressive On/off
      alrmProgVol[alrmNum]=!alrmProgVol[alrmNum]; //Toggle Progressive Alarm Vol
      delay (5);
      EEPROM.write (alarmProgVolLoc[alrmNum],alrmProgVol[alrmNum]);
      cls();
      break;
  }

}

// =======================================================================================
// ---- Show Alarm setting screen ----
// By: LensDigital
// =======================================================================================
void showAlarm(byte color){
  if (!isSettingAlarm) return; // Exit if not setting alarm
  byte alrmNum;
  char myString[11];
  alrmBlink(color);
  if (menuItem==1) alrmNum=0; // Decide which Alarm to show
  else alrmNum=1;
  switch (subMenu[alrmNum]) { 
    
    case 1: // Alarm On/off
      snprintf(myString,sizeof(myString),"Alrm%d",alrmNum+1);
      showText(1,8,myString,1,color);
      if (alarmon[alrmNum] & 128) showText(10,0,"ON",1,hhColor); 
      else showText(10,0,"OFF",1,hhColor); 
      break;

    case 2:  // Alarm Frequency Setup (Off/Daily/Weekday/Custom)
      snprintf(myString,sizeof(myString),"Alrm%d",alrmNum+1);
      showText(1,8,myString,1,color);
      switch (alarmon[alrmNum]) {
        case 0:
          showText(10,0,"ERR",1,hhColor); // Should never come here!
          break;
          case 255:
          showText(0,0,"Daily",1,hhColor);
          break;
        case 252:
          showText(1,0,"WKDAY",1,hhColor);
          break;
        default:
          showText(0,0,"Custm",1,hhColor);
     }
  
    case 3: // Custom Alarm day setting
      snprintf(myString,sizeof(myString),"Alrm%d",alrmNum+1);
      showText(1,8,myString,1,color);
      switch (subMenu[4+alrmNum]){
        case 1: // Monday
          showText(0,0,"Mon:",1,color);
          if (alarmon[alrmNum] & 64) showText(25,0,"Y",1,hhColor) ;
          else showText(25,0,"N",1,hhColor); 
          break;
        case 2: // Tue
          showText(0,0,"Tue:",1,color);
          if (alarmon[alrmNum] & 32) showText(25,0,"Y",1,hhColor);
          else showText(25,0,"N",1,hhColor);
          break;
        case 3: // Wed
          showText(0,0,"Wed:",1,color);
          if (alarmon[alrmNum] & 16) showText(25,0,"Y",1,hhColor);
          else showText(25,0,"N",1,hhColor);
          break;
        case 4: // Thu
          showText(0,0,"Thu:",1,color);
          if (alarmon[alrmNum] & 8) showText(25,0,"Y",1,hhColor);
          else showText(25,0,"N",1,hhColor);
          break;
        case 5: // Fri
          showText(0,0,"Fri:",1,color);
          if (alarmon[alrmNum] & 4) showText(25,0,"Y",1,hhColor);
          else showText(25,0,"N",1,hhColor);
          break;
        case 6: // Sat
          showText(0,0,"Sat:",1,color);
          if (alarmon[alrmNum] & 2) showText(25,0,"Y",1,hhColor);
          else showText(25,0,"N",1,hhColor);
          break;
        case 7: // Sun
          showText(0,0,"Sun:",1,color);
          if (alarmon[alrmNum] & 1) showText(25,0,"Y",1,hhColor);
          else showText(25,0,"N",1,hhColor);
          break;
        case 8: // Exit
          showText(0,0,"Exit",1,color);
          break;
      }
      break;

    case 4: // Setting Alarm hours and minutes
    case 5:
      snprintf(myString,sizeof(myString),"Alrm%d",alrmNum+1);
      showText(1,8,myString,1,color);
      if(time12hr) {
        // == BEGIN 12 Hour Mode ====
        myhours=alrmHH[alrmNum]; // Get Alarm Hours from EEPROM
        if (myhours==0) myhours=12; // Midnight
        else if (myhours >12) myhours=myhours-12;
//        if (alrmHH[alrmNum]<12)  plot (0,1,hhColor); // Show AM Dot
        if (alrmHH[alrmNum]>=12)  plot (0,1,hhColor); // Show PM Dot (wbp)
        else plot (0,1,BLACK); // Hide PM Dot
        snprintf(myString,sizeof(myString), "%d",myhours);
        if ( (myhours/10)%10 ==0) {// It's one digit hour so need to hide first digit
          showText(1,0," ",1,hhColor);
          showText(7,0,myString,1,hhColor);
        }
        else
          showText(1,0,myString,1,hhColor);
      }
      // === END 12 Hour Mode ===
      else {
         //Serial.println ("We are in 24 Hour Mode");
        // 24 Hour Mode
        //plot (1,1,BLACK); // Hide AM Dot
        snprintf(myString,sizeof(myString), "%02d",alrmHH[alrmNum]); // make 2 digit hours
        showText(1,0,myString,1,hhColor);
      }
      snprintf(myString,sizeof(myString), "%02d",alrmMM[alrmNum]);
      showText(12,0,":",1,color); // Show colum :
      showText(18,0,myString,1,mmColor);
      
      break;

    case 6: // Alarm Tone selection
      showText(1,0,"Tone:",1,color);
      snprintf(myString,sizeof(myString), "ALRM%d",alrmToneNum[alrmNum]); // Make Alarm Filename
      showText(0,8,myString,1,hhColor); 
      break;

    case 7: // Alarm Progressive Volume On/off
      showText(1,0,"P.Vol:",1,color);
      if (alrmProgVol[alrmNum]) showText(10,8,"ON",1,hhColor); 
      else showText(10,8,"OFF",1,hhColor); 
      break;
    
  }
  
}

// =======================================================================================
// ---- Alarm Setting Blink Function ----
// ---- by LensDigital
// =======================================================================================
void alrmBlink(byte color) {
  int blinkDigDuration =500;
     if (blinking) {
       if ( (millis()-blinkTime > blinkDigDuration)) { // It's been over blinkDuration time
          blinkTime = millis(); // reset offset to current time
          if (isSettingAlrmHH) {
            if (hhColor==BLACK) {
             hhColor = color; // Inverse color of Hours 
             }
           else {
             hhColor = BLACK;
            
           }
             
           mmColor = color; // Minutes not blinking
          }
          if (isSettingAlrmMM) {
           if ( mmColor == BLACK ) mmColor = color; // Inverse color of Minutes 
           else mmColor = BLACK;
           hhColor = color; // Hours not blinking
          }
        } 
    } 
    else {  // DO not blink!
     if (isSettingAlrmHH) {
       hhColor=color;
       mmColor=color;
     }
     else {
       hhColor=color; 
       mmColor=color;
     }
    }
    
}

// =======================================================================================
// ---- Show  DST Menu (Daylight Savings Time) ----
// ---- by wbphelps
// =======================================================================================
void showDST(byte color) {
  if (!isSettingDST) return; // Exit if not setting DST
  showText(6,0,"DST",1,color);
  switch (g_DST_mode) {
    case 0: // DST off
    showText(6,8,"Off ",1,color);
    break;
    case 1: // DST on
    showText(6,8,"On  ",1,color);
    break;
    case 2: // DST Auto
    showText(6,8,"Auto",1,color);
    break;
   }
}

bool isLeapYear(int year)
{
  bool ly = false;
  if ((year % 4) == 0) {
    if ((year % 100) == 0) {
      ly = ((year % 400) == 0);
     }
    else {
      ly = true;
    }
  }
  return(ly);
}

// =======================================================================================
// ---- Set TimeDate ----
// ---- by LensDigital
// =======================================================================================
//const uint8_t mDays[]={31,28,31,30,31,30,31,31,30,31,30,31};
void setTimeDate() {
  time_t t = now(); // get date/time atomicly (wbp)
  hours=hour(t); // Store current hour value from clock
  minutes=minute(t); // Store current minute value from clock
  seconds=second(t);
  months=month(t); // Store current month value from clock
  days=day(t);// Store current day value from clock
  years=year(t) %100; // Store current year (last 2 digits) from clock
  byte mdays = mDays[months-1];  // number of days in current month
  if (months == 2)
    if (isLeapYear(year(t)))
      mdays = 29;
  switch (subMenu[2]) {
    case 1: // Set hours
      if (decrement) hours--;
      else hours++;
      if (hours == 255) hours = 23;
      else if (hours > 23) hours = 0;
      break;
    case 2: // Set minutes
      if (decrement) minutes--;
      else minutes++;
      if (minutes == 255) minutes = 59;
      else if (minutes > 59) minutes = 0;
      seconds=0; // Reset seconds to 0 with each minute change
      break;
    case 3: // Set months
       if (decrement) months--;
       else months++;
       if (months == 0) months=12;
      else if (months > 12) months=1;
      break;
    case 4: // Set Days
      if (decrement) days--;
      else  days++;
      if (days == 255) days = 31;
      else if (days > mdays)  days = 1;
      break;
    case 5: // Set Years 
      if (decrement) years--;
      else years++;
      if (years < 10 ) years = 40;
      else if (years > 40) years = 10; // Default to 2010
      break;
//    case 6: // Set DST +1 hours 
//      playSFX(1);
//      hours++;
//      if (hours == 255) hours = 23;
//      else if (hours > 23) hours = 0;
//      break;
//   case 7: // Set DST -1 hours 
//      playSFX(1);
//      hours--;
//      if (hours == 255) hours = 23;
//      else if (hours > 23) hours = 0;
//      break;
    case 6: // DST
      playSFX(1);
      g_DST_mode = ++g_DST_mode % 3;  // off, on, Auto
      if (g_DST_mode == 1) { // on
        hours++;  // spring ahead
        g_DST_offset = 1;  // remember offset
      }
      else { // off or Auto
        if (g_DST_offset) { // was it set?
          hours--;  // fall back
          g_DST_offset = 0;
        }
      }
      if (hours == 255)  hours = 23;  // handle wrap
      else if (hours > 23)  hours = 0;
//      g_DST_updated = false; // re-init & re-calc DST for today
      EEPROM.write(DSTmodeLoc,g_DST_mode);
      break;
    }  
   // IMPORTANT! This will keep track of seconds for better time setting! 
   if ( millis()-last_ms > 1000) { // Has it been over 1 second since Set button was pressed?
      seconds++;
      if (seconds >59) {
          seconds=0;
          minutes++;
      }
      if (minutes > 59) {
          minutes=0;
          hours++;
      }
      if (hours >23){
          hours =0;
          days++;
      }
     last_ms=millis();   
    }
    setTime(hours,minutes,seconds,days,months,years); // Sets System time (and converts year string to integer)
    RTC.set(now()); // Writes time change to RTC chip     
    g_DST_updated = false; // re-init & re-calc DST for today
}


// =======================================================================================
// ---- System Settings ----
// ---- by LensDigital
// =======================================================================================
void sysSetting(){
 //putstring_nl ("Setting System");
 if (subMenu[3]) playSFX(1); // Don't play sound if not setting anything, i.e. submenu=0
 switch (subMenu[3]) {
    case 1: // 12/24 Hour Mode
        if (!time12hr) time12hr=true; 
        else time12hr=false ; 
        EEPROM.write(mode24HRLoc,time12hr);
        delay (5);
         break;
     case 2: // Set Display Brightness
        cls();
        if (decrement) brightness--;
        else brightness++;
        if (brightness == 255) brightness = 5;
        else if (brightness > 5) brightness = AUTO_BRIGHTNESS_ON;  // 0 if photocell present, else 1
        EEPROM.write(brightLoc,brightness);
        delay (15);
        prevBrightness=0; // force an update
        if (brightness==0) autoBrightness();
        else
          setBrightness(brightness);
        break;
     case 3: // Set Clock Color
       cls();
       if (autoColor)
         autoColor = false;
       else if (decrement) {
         if (clockColor>1)
           clockColor--;
         else {
           clockColor = 3;
           autoColor = true;
         }
       }
       else {
         if (clockColor<3)
           clockColor++;
         else {
           clockColor = 1;
           autoColor = true;
         }
       }
       EEPROM.write(clockColorLoc,clockColor);
       delay (5);
       EEPROM.write(autoColorLoc,autoColor);
       delay (5);
       break;
     case 4: // Set Clock Font (0-5)
       cls();
       if (decrement) clockFont--;
       else clockFont++;
       if (clockFont ==255) clockFont=4;
       else if (clockFont>4) clockFont=0;
       EEPROM.write(clockFontLoc,clockFont);
       delay (5);
       break;
     case 5: // Set sFX on/off
       cls();
       if (sFX) sFX=false;
       else  sFX=true;
       EEPROM.write(sFXLoc,sFX);
       delay (5);
       break;
     case 6: // Temperature Units
       cls();
       if (tempUnit) tempUnit=false;
       else  tempUnit=true;
       EEPROM.write(tempUnitLoc,tempUnit);
       delay (5);
       break;
      case 7: // Sound Volume
        cls();
        if (decrement) sndVol++;
        else sndVol--;
        if (sndVol == 255) sndVol=8;
        else if (sndVol > 8) sndVol=0;
        //playSFX(1);
        EEPROM.write(sndVolLoc,sndVol);
        delay (5);
        break;
      case 8: // Startup on/off
        cls();
        if (doStartup) doStartup=false;
        else doStartup=true;
        //playSFX(1);
        EEPROM.write(doStartupLoc,doStartup);
        delay (5);
        break;
      case 9: // RFM
        cls();
        if ( RFM12B_Enabled ) { // If chip phisically present
          if (isRadioPresent) isRadioPresent=false;
          else {
            isRadioPresent=true;
            delay (15);
          }
          //playSFX(1);
          EEPROM.write(radioOnLoc,isRadioPresent);
          delay (5);
        }
        else {
          playSFX(4);
          isRadioPresent=false;
        }
      break;
      case 10: // IR
        cls(); 
        if (IR_PRESENT) { // If IR Hardware defined globally
          if (isIRPresent) isIRPresent=false; // Turn off IR
          else isIRPresent=true; // Turn on IR
          //playSFX(1);
        }
        EEPROM.write(IROnLoc,isIRPresent); 
        delay (5);
      break;
      case 11: // GPS
        cls(); 
        if (GPS_PRESENT) { // If GPS Hardware defined globally
          if (isGPSPresent) isGPSPresent=false; // Turn off GPS
          else isGPSPresent=true; // Turn on GPS
          //playSFX(1);
        }
        EEPROM.write(GPSOnLoc,isGPSPresent); 
        delay (5);
      break;
  }

}

// =======================================================================================
// ---- Show System setting screen ----
// By: LensDigital
// =======================================================================================
void showSys(){
  if (!isSettingSys) return; // Exit if not setting system
  byte color=clockColor;
  int blinkDigDuration =500;
  if ( (millis()-blinkTime > blinkDigDuration)) { // It's been over blinkDuration time
      blinkTime = millis(); // reset offset to current time
      if ( hhColor == BLACK ) hhColor = color; // Inverse color  
      else hhColor = BLACK;
  } 
  switch (subMenu[3]){ 
    case 1: // We are setting 12/24 Hour mode
      showText(1,0,"Mode:",1,color);
      if (time12hr) showText(1,8,"12 HR ",1,hhColor);
      else showText(1,8,"24 HR",1,hhColor);
      break;
    case 2:  // Adjust Brightness Level
      showText(1,0,"Light",1,color);
      //Serial.println (brightness);
      switch (brightness) {
      case 0:
        showText(3,8,"Auto",1,hhColor);
        break;
      case 1:
        showText(3,8,"Night",1,hhColor);
        break;
      case 2:
        showText(3,8,"Low",1,hhColor);
        break;
      case 3:
        showText(3,8,"Med",1,hhColor);
        break;
      case 4: 
        showText(3,8,"High",1,hhColor);
        break;
      case 5: 
        showText(3,8,"Max",1,hhColor);
        break;
      }
    break;
    case 3: // Set Clock Color
      showText(1,0,"Color",1,color);
      if (autoColor)
        showText(1,8,"Auto",1,ORANGE);
      else switch ( clockColor) {
       case 1:  
         showText(8,8,"Red",1,RED);
         break;  
       case 2:  
         showText(1,8,"Green",1,GREEN);
         break;
       case 3:  
         showText(1,8,"Yello",1,ORANGE);
         break;
      }
    break;
    case 4: // Set Clock Font
      char myString[8];
      showText(1,0,"Font:",1,color);
      snprintf(myString,sizeof(myString), "Font%d",clockFont); // Show current font number
      showText(1,8,myString,1,hhColor);
    break;
    case 5: // Enable/Disable menu SFX
      showText(2,0,"Menu",1,color);
      showText(0,8,"sFX",1,color);
      if (sFX) showText(18,8,"ON",3,hhColor);
      else showText(18,8,"OFF",3,hhColor);
    break;
    case 6: // Temperature C or F
      showText(2,0,"Temp",1,color);
      if (tempUnit) showText(10,8,"F",1,hhColor);
      else  showText(10,8,"C",1,hhColor);
      ht1632_putchar(18,8,127,color); // Show Degree Sign
      break;
    case 7: // Sound Volume
      byte barColor; // Color of volume bar
      showText(0,0,"Sound",1,color);
      showText(2,9,"VOL",1,color);
      for (int y=8;y>sndVol;y--)
         for (int x=0;x<y-sndVol;x++)
          plot ( (x+24)+(8-y),(y+8)-1, color); 
      break; 
    case 8: // Startup on/off
      showText(2,0,"Startup",3,color);
      if (doStartup) showText(10,8,"ON",1,hhColor); 
      else showText(10,8,"OFF",1,hhColor); 
    break;
    case 9: // RF Module
      showText(2,0,"RFM12",1,color); 
      if (isRadioPresent) showText(10,8,"ON",1,hhColor); 
      else showText(10,8,"OFF",1,hhColor);  
    break;
    case 10: // IR Receiver
      showText(2,0,"IR",1,color); 
      if (isIRPresent) showText(10,8,"ON",1,hhColor); 
      else showText(10,8,"OFF",1,hhColor);  
    break;
    case 11: // GPS Receiver
      showText(2,0,"GPS",1,color); 
      if (isGPSPresent) showText(10,8,"ON",1,hhColor); 
      else showText(10,8,"OFF",1,hhColor);  
    break;
  }
}


// =======================================================================================
// ---- User Option Settings ----
// ---- by LensDigital
// =======================================================================================
void optSetting(){
 if (subMenu[6]) playSFX(1); // Don't play sound if not setting anything, i.e. submenu=0
 switch (subMenu[6]) {
   case 1: // InfoDisplay
     switch (subMenu[7]) {
       case 0: // Enter Menu
         cls();
         subMenu[7]=1;
         //putstring_nl ("Enter talk Submenu");
         break;
       case 1: // Frequency
         if (decrement) infoFreq--;
         else infoFreq++;
         if (infoFreq == 0) infoFreq=9;
         else if (infoFreq > 9) infoFreq=0;
         EEPROM.write(infoFreqLoc,infoFreq);
         delay (5);
         cls();
       break;
       case 2: // Scroll Date
         putstring_nl ("Scroll Date");
         infoOptions = infoOptions ^ 128; //Toggle
         EEPROM.write (infoOptionsLoc,infoOptions);
         delay (5);   
        break;
       case 3: // Scroll inernal temp
         putstring_nl ("Scroll Temp");
         infoOptions = infoOptions ^ 64; //Toggle
         EEPROM.write (infoOptionsLoc,infoOptions);
         delay (5);   
         break;
       case 4: // Scroll External Temp
         if ( RFM12B_Enabled ) {
           infoOptions = infoOptions ^ 32; //Toggle
           EEPROM.write (infoOptionsLoc,infoOptions);
           delay (5);   
         } 
         else playSFX(4);
         break;
       case 5: // Scroll  Alarm
         infoOptions = infoOptions ^ 16; //Toggle
         EEPROM.write (infoOptionsLoc,infoOptions);
         delay (5);   
         break;
       case 6: // Scroll  Sensor Data
       if ( RFM12B_Enabled ) {
         infoOptions = infoOptions ^ 8; //Toggle
         EEPROM.write (infoOptionsLoc,infoOptions);
         delay (5);   
       }
       else playSFX(4);
       break;
       case 7: // Scroll  Humidity data
       if ( RFM12B_Enabled ) {
         infoOptions = infoOptions ^ 4; //Toggle
         EEPROM.write (infoOptionsLoc,infoOptions);
         delay (5);   
       }
       else playSFX(4);
       break;
       case 8: // Exit
         subMenu[7]=0;
       break;
     }
     
   break;
   case 2: // Say Items Toggle
     switch (subMenu[8]) {
       case 0: // Enter Menu
         cls();
         subMenu[8]=1;
         //putstring_nl ("Enter talk Submenu");
         break;
       case 1: // Say Time
         sayOptions = sayOptions ^ 64; //Toggle
         EEPROM.write (sayOptionsLoc,sayOptions);
         delay (5);   
         break;
      case 2: // Say Date
         sayOptions = sayOptions ^ 32; //Toggle
         EEPROM.write (sayOptionsLoc,sayOptions);
         delay (5);   
         break;
      case 3: // Say Internal Temp
         sayOptions = sayOptions ^ 16; //Toggle
         EEPROM.write (sayOptionsLoc,sayOptions);
         delay (5);   
         break;
      case 4: // Say External Temp
         //Serial.println ("Saving say Ext Temp");
         if ( RFM12B_Enabled ) {
           sayOptions = sayOptions ^ 4; //Toggle
           EEPROM.write (sayOptionsLoc,sayOptions);
           delay (10);   
         }
         else playSFX(4);
         break;
     case 5: // Say External Humidity
         if ( RFM12B_Enabled ) {
           sayOptions = sayOptions ^ 2; //Toggle
           EEPROM.write (sayOptionsLoc,sayOptions);
           delay (10);   
         }
         else playSFX(4);
         break;
      case 6: // Say  Alarm
         
         sayOptions = sayOptions ^ 8; //Toggle
         EEPROM.write (sayOptionsLoc,sayOptions);
         delay (10);   
         break;
      case 7: // Exit
         subMenu[8]=0;
         break;
     }
  break;
  case 3: // temperature offset
     if (decrement) tmpOffset--;
     else tmpOffset++;
     if (tmpOffset == 255) tmpOffset=9;
     else if (tmpOffset>9) tmpOffset=0;
     EEPROM.write (tmpOffsetLoc,tmpOffset);
     delay (5);
     cls();
   break;
 
 }
 
}

// =======================================================================================
// ---- Show User Options setting screen ----
// By: LensDigital
// =======================================================================================
void showOpt(){
  if (!isSettingOptions) return; // Exit if not setting options
  byte color=clockColor;
  char myString[2]; 
  int blinkDigDuration =500;
  if ( (millis()-blinkTime > blinkDigDuration)) { // It's been over blinkDuration time
      blinkTime = millis(); // reset offset to current time
      if ( hhColor == BLACK ) hhColor = color; // Inverse color  
      else hhColor = BLACK;
  } 
  switch (subMenu[6]){ 
   case 1: // InfoDisplay
      if (subMenu[7]==0) { // Will skip showing this text if we are deeper in submenu
       showText(0,0,"Info",1,color);
       showText(0,8,"Displ",1,color);
       }
     switch (subMenu[7]){  
     case 1: // Frequency
       showText(1,0,"Show:",1,color);
       if (infoFreq==0) showText(1,8,"Never",1,hhColor);
       else {
        showText(0,8,"Every:",3,color);
        snprintf(myString,sizeof(myString), "%d",infoFreq); // Make string
        showText(24,8,myString,3,hhColor);
       }
      break;
      case 2: // Scroll Date
       showText(0,0,"Displ",1,color);
       showText(0,8,"Date",1,color);
       if (infoOptions & 128) showText(25,8,"Y",1,hhColor) ;
       else showText(25,8,"N",1,hhColor); 
      break;
      case 3: // Scroll Inside Temp
       showText(0,0,"Displ",1,color);
       showText(0,8,"InTemp",3,color);
       if (infoOptions & 64) showText(25,8,"Y",1,hhColor) ;
       else showText(25,8,"N",1,hhColor);
      break;
      case 4: // Scroll Outside Temp
       if (!isRadioPresent) { // Skip if Receiver is not enabled
         subMenu[7]++;
         break;
       }
       showText(0,0,"Displ",1,color);
       showText(0,8,"ExTemp",3,color);
       if (infoOptions & 32) showText(25,8,"Y",1,hhColor) ;
       else showText(25,8,"N",1,hhColor);
       break;
       case 5: // Scroll Alarm
       showText(0,0,"Displ",1,color);
       showText(0,8,"Alarm",3,color);
       if (infoOptions & 16) showText(25,8,"Y",1,hhColor) ;
       else showText(25,8,"N",1,hhColor);
      break;
      case 6: // Scroll receiver stats
       if (!isRadioPresent) { // Skip if Receiver is not enabled
         subMenu[7]++;
         break;
       }
       showText(0,0,"Displ",1,color);
       showText(0,8,"RFStat",3,color);
       if (infoOptions & 8) showText(25,8,"Y",1,hhColor) ;
       else showText(25,8,"N",1,hhColor);
       break;
       case 7: // Scroll humidity
      if (!isRadioPresent) { // Skip if Receiver is not enabled
         subMenu[7]++;
         break;
       }
       showText(0,0,"Displ",1,color);
       showText(0,8,"Humid",3,color);
       if (infoOptions & 4) showText(25,8,"Y",1,hhColor) ;
       else showText(25,8,"N",1,hhColor);
      break;
      case 8: // Exit
      showText(0,0,"Exit",1,color);
      break;
     }
   break;
   case 2: // Talk Items selection (Submenu 7)
     
     if (subMenu[8]==0) { // Will skip showing this text if we are deeper in submenu
       showText(0,0,"Talk",1,color);
       showText(0,8,"Items",1,color);
     }
     switch (subMenu[8]){
     case 1: // Say Time
       showText(0,0,"Say",1,color);
       showText(0,8,"Time",1,color);
       if (sayOptions & 64) showText(25,8,"Y",1,hhColor) ;
       else showText(25,8,"N",1,hhColor); 
     break;
     case 2: // Say Date
       showText(0,0,"Say",1,color);
       showText(0,8,"Date",1,color);
       if (sayOptions & 32) showText(25,8,"Y",1,hhColor) ;
       else showText(25,8,"N",1,hhColor); 
     break;
     case 3: // Say Inside Temperature
       showText(0,0,"Say",1,color);
       showText(0,8,"InTemp",3,color);
       if (sayOptions & 16) showText(25,8,"Y",1,hhColor) ;
       else showText(25,8,"N",1,hhColor); 
     break;
     case 4: // Say Outside Temperature
       if (!isRadioPresent) { // Skip if Receiver is not enabled
         subMenu[8]++;
         break;
       }
       showText(0,0,"Say",1,color);
       showText(0,8,"ExTemp",3,color);
       if (sayOptions & 4) showText(25,8,"Y",1,hhColor) ;
       else showText(25,8,"N",1,hhColor); 
       break;
     case 5: // Say Humidity
       if (!isRadioPresent) { // Skip if Receiver is not enabled
         subMenu[8]++;
         break;
       }
       showText(0,0,"Say",1,color);
       showText(0,8,"Humid",3,color);
       if (sayOptions & 2) showText(25,8,"Y",1,hhColor) ;
       else showText(25,8,"N",1,hhColor); 
       break;
     case 6: // Say Alarm
       showText(0,0,"Say",1,color);
       showText(0,8,"Alrm",1,color);
       if (sayOptions & 8) showText(25,8,"Y",1,hhColor) ;
       else showText(25,8,"N",1,hhColor); 
       break;
     case 7: // Exit
       showText(0,0,"Exit",1,color);
       break;
     }
   break;
   case 3: // Temperature offset
     showText(0,0,"Temp",1,color);
     showText(0,8,"Offs:-",3,color);
     snprintf(myString,sizeof(myString), "%d",tmpOffset); // Make string
     showText(25,8,myString,1,hhColor);
   break;
  }
}

// =======================================================================================
// ---- Check Incr button to show Quick Menu ----
// ---- Press and quickly release to go thru one item at a time
// ---- Hold button for more than a second to autoamtically go thru all items.
// ---- by LensDigital
// =======================================================================================
void quickMenu(){
  // check MENU/INCR button;
    currStatusInc=digitalRead(INC_BUTTON_PIN);
    if (currStatusInc!=lastStatusInc) {// Change Occured 
       lastStatusInc=currStatusInc;
       if (isInMenu) return; //Make sure we are not in settings menu
       quickDisplay(); // Process Quick Menu functions
    }
  if (isInQMenu)
    if ( (digitalRead(SET_BUTTON_PIN) == HIGH) || (digitalRead(MENU_BUTTON_PIN) == HIGH) ){
       lastButtonTime = 0; // Exit Quick Menu if any button other than INC was pres
      //processSetButton(); // Only to control alarm
    }
   // display the menu option for 10 sec after menu button was pressed;
  if ((lastButtonTime > 0) && (millis() - lastButtonTime < 10000))  // wbp
    return;
  
  // Finished with menus, return to normal operations
  if (isInQMenu) {
    putstring_nl ("Exiting QMenu");
    cls ();
    okClock=true; // Enable Big Clock display
    isSettingDate=false;
    isInQMenu=false;
    mbutState=1; // Reset Menu Button State
    isIncrementing = false;
    blinking = false;
    isSettingAlarm=false;
    isSettingDST=false; // wbp
    menuItem=0; 
    subMenu[0]=0;
    subMenu[1]=0;
    mbutState=1;
    cls();
  }
}

// =======================================================================================
// ---- Play menu sounds (sFX) ----
// ---- Pass 1 for menu naviagtion, 2 chnging item , 3 for exit, 4 for error
// ---- by LensDigital
// =======================================================================================
void playSFX(byte item){
  if (!sFX) return; // Global Sound Effects are off
  turnOffRadio(); // Disable RF12B
  switch (item) {
   case 1: 
    playfile("MENU1.WAV");
    break;
   case 2:
    playfile("MENU2.WAV"); 
    break;
   case 3: 
    playfile("MENU3.WAV");
    break;
  case 4: 
    playfile("ERR1.WAV");
    break;    
  }
  //radio.Wakeup(); // Disable RF12B
}

// =======================================================================================
// ---- Write ALarm HH:MM changes to EEProm ----
// ---- Using deidcated function to slow wear down of EEProm 
// ---- by LensDigital
// =======================================================================================
void writeEEPROMAlrm (byte alrmNum) {
  //putstring_nl ("Write Alarm to EEProm");
  EEPROM.write(alarmHHLoc[alrmNum],alrmHH[alrmNum]);
  delay (15);
  EEPROM.write(alarmMMLoc[alrmNum],alrmMM[alrmNum]);
  delay (15);
  isAlarmModified[alrmNum]=false;
}

// =======================================================================================
// ---- Helper Process for Set Button for Talking Setting ----
// ---- by Bratan
// =======================================================================================
void userMenu () {
     cls ();
     isSettingAlarm = false;
     isSettingTime   = false;
     isSettingSys=false;
     isSettingOptions=true;
     if (subMenu[6]==1) { // Do not increment this submenu (we are in Infodisplay Options menu)
        //putstring_nl ("In subMenu");
       if (subMenu[7] == 0) subMenu[6]++; // Go to next item, we are not enterying this submenu tree
       else {
         subMenu[7]++;
         if (subMenu[7] > 8) subMenu[7]=1; // Goes back to first item of this submenu 
       }
     }
     else if (subMenu[6]==2) { // Do not increment this submenu (we are in Talking Options menu)
              //putstring_nl ("In subMenu");
             if (subMenu[8] == 0) subMenu[6]++; // Go to next item, we are not enterying this submenu tree
             else {
               subMenu[8]++;
               if (subMenu[8] > 7) subMenu[8]=1; // Goes back to first item of this submenu 
             }
     } 
     else subMenu[6]++; // Increment button press count
     if (subMenu[6] > 3) subMenu[6]=1; // Goes back to first menu item
     cls ();
}

// =======================================================================================
// ---- Helper Process for Set Button for Time/Date Setting ----
// ---- by Bratan
// =======================================================================================
void butSetClock() {
  cls();
        startBlinking();
        isSettingAlarm = false;
        isSettingTime   = true;
        isSettingSys=false;
        subMenu[2]++; // Increment button press count
//        if (subMenu[2] > 7) subMenu[2]=1; // Goes back to first menu item
        if (subMenu[2] > MAX_SETTINGS) subMenu[2]=1; // Goes back to first menu item (wbp)
      	switch (subMenu[2]) {
         case 1: // Set hours
          //putstring_nl ("SET: Hours");
          cls();
          okClock=true;
          isSettingTime   = true;
          isSettingDate = false;
          isSettingHours   = true;
	  isSettingMinutes = false;
	  isSettingMonth   = false;
	  isSettingDay   = false;
	  isSettingYear   = false;
          isSettingDST = false;
          break;
         case 2: // Set Minutes
          //putstring_nl ("SET: Mins");
          isSettingHours   = false;
	  isSettingMinutes = true;
	  isSettingMonth   = false;
	  isSettingDay   = false;
	  isSettingYear   = false;
          break;
         case 3: // Set Month  (wbp)
          isSettingDate = true;
          okClock=false;
          cls();
          //putstring_nl ("SET: Month");
          isSettingHours   = false;
      	  isSettingMinutes = false;
      	  isSettingMonth   = true;
      	  isSettingDay   = false;
      	  isSettingYear   = false;
          break;
         case 4: // Set Day
          //putstring_nl ("SET: Day");
          isSettingHours   = false;
	  isSettingMinutes = false;
	  isSettingMonth   = false;
	  isSettingDay   = true;
	  isSettingYear   = false;
          break;
         case 5: // Set year
          //putstring_nl ("SET: Year");
          isSettingHours   = false;
	  isSettingMinutes = false;
	  isSettingMonth   = false;
	  isSettingDay   = false;
	  isSettingYear   = true;
          break;
         case 6: // DST on/off/auto
          isSettingDST = true;
          isSettingDate = false;
          isSettingYear   = false;
          break;
        }
}

// =======================================================================================
// ---- Helper Process for Set Button for Alarm Setting ----
// ---- by Bratan
// =======================================================================================
void butSetAlarm (byte alrmNum) {
     startBlinking();
     //putstring_nl ("SET: Alarm");
     cls();
     isSettingAlarm = true;
     isSettingSys=false;
     if (subMenu[alrmNum]==3) ; // Do not increment
     else subMenu[alrmNum]++; // Increment button press count
     if (subMenu[alrmNum] > 7) subMenu[alrmNum]=1; // Goes back to first menu item  (wbp)
     if (!(alarmon[alrmNum] & 128) && subMenu[alrmNum]==2)  subMenu[alrmNum]=4; // ALarm is off so we need to skip 4nd menu
     if (!isSettingAlrmCust[alrmNum] && subMenu[alrmNum]==3) subMenu[alrmNum]=4; // Custom Alarm is not set so skip to 4th menu
      switch (subMenu[alrmNum]) {
         case 1: // Set Alarm on Off
           cls();
           isSettingDate = false;
           isSettingAlarm = true;
           isSettingAlrmHH = true;
           break;
         case 2: // SET: Alarm Dalily/Weekday/Custom
           //putstring_nl ("SET: Alarm OFF/Dalily/Weekday/Custom");
           cls();
           isSettingDate = false;
           isSettingAlarm = true;
           isSettingAlrmHH = true;
           break;
          case 3: // Set Alarm Custom schedule
            if (isSettingAlrmCust[alrmNum]) {
             //putstring_nl ("SET: Alarm1. Custom Schedule");
             subMenu[4+alrmNum]++;
             if (subMenu[4+alrmNum] > 8) subMenu[4+alrmNum]=1; // Goes back to first menu item (Monday)
           }
           isSettingAlrmHH = true;
           isSettingAlrmMM = false;
           break;
          case 4: // Set Alarm Hrs
          //putstring_nl ("SET: Alarm HRS");
           cls();
           isSettingAlrmHH = true;
           isSettingAlrmMM = false;
          break;
          case 5: // Set Alarm min
          //putstring_nl ("SET: Alarm MIN");
           isSettingAlrmHH = false;
           isSettingAlrmMM = true;
           break; 
         case 6: // Set Alarm Tone
         //putstring_nl ("SET: Alarm Tone");
           isSettingAlrmHH = true; 
           isSettingAlrmMM = false; 
           break;
        }
}

