// =======================================================================================
// ---- Show Menu items and set vars ----
// ---- by LensDigital
// =======================================================================================
void showMenu() 
{
  playSFX(2);
  for (int i=0;i<MAX_SUBMENUS;i++) { // Initialize SubMenus  (wbp)
      subMenu[i]=0;
   }
     
  switch (menuItem) {
    case 1: // Set Alarm 1 Menu Item (Display Only)
      //putstring_nl ("Display Alrm 1 Menu");
      okClock=false;
      isSettingSys=false;
      isSettingOptions=false;
      cls();
      showText(1,0,"Set",1,ORANGE);
      showText(3,8,"Alrm1",1,ORANGE);
      break;
    case 2: // Set Alarm 2 Menu Item (Display Only)
      //putstring_nl ("Display Alrm 2 Menu");
      okClock=false;
      isSettingSys=false;
      cls();
      showText(1,0,"Set",1,ORANGE);
      showText(3,8,"Alrm2",1,ORANGE);
      break;
    case 3: // Time and Date setting menu
      //putstring_nl ("Display Date/Time Menu");
      okClock=false;
      isSettingSys=false;
      cls();
      showText(1,0,"Time/",1,ORANGE);
      showText(5,8,"Date",1,ORANGE);
      break;
   case 4: // System Settings (preferences/setup)
      //putstring_nl ("Display System Menu");
      okClock=false;
      cls();
      showText(1,0,"Sys",1,ORANGE);
      showText(3,8,"Setup",1,ORANGE);
      break;
   case 5: // User Options menu
      //putstring_nl ("Display Options Menu");
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
  alarmState[alrmNum] = AS_OFF; // Make sure alarm that just sounded doesn't resume
  switch (subMenu[alrmNum]) {
    case 1: // Alarm On/off
      Settings.alarmOn[alrmNum]=Settings.alarmOn[alrmNum] ^ 128; //Toggle first bit (on/off)
      cls();
      playSFX(1);
      break;
    case 2: // Set Alarm frequency to off/daily/weekday/custom
      switch (Settings.alarmOn[alrmNum]){
        case 255:
          Settings.alarmOn[alrmNum]=252; // Was Daily, now Workday
          Settings.alarmCustom[alrmNum] = false;
          break; 
        case 252:
          Settings.alarmOn[alrmNum]=254; // Was Workday, now custom
          Settings.alarmCustom[alrmNum] = true;
          break;
        default:
          Settings.alarmOn[alrmNum]=255; // Was custom, now Daily
          Settings.alarmCustom[alrmNum] = false; // Remove if want to keep custom Menu setting even if alarm is off (need to tweak to make cusotm right after OFF so it will remember day settings 
        }
      playSFX(1);
      cls();
      break;
    case 3: // Custom Day selection
      // Check if Custom Seeting was picked, if not skip this option
      switch (subMenu[4+alrmNum]) {
        case 1: // Mon
          Settings.alarmOn[alrmNum]=Settings.alarmOn[alrmNum] ^ 64; //Toggle
          break;
        case 2: // Tue
          Settings.alarmOn[alrmNum]=Settings.alarmOn[alrmNum] ^ 32; //Toggle 
          break;
        case 3: // Wed
          Settings.alarmOn[alrmNum]=Settings.alarmOn[alrmNum] ^ 16; //Toggle 
          break;
        case 4: // Thu
          Settings.alarmOn[alrmNum]=Settings.alarmOn[alrmNum] ^ 8; //Toggle 
          break;
        case 5: // Fri
          Settings.alarmOn[alrmNum]=Settings.alarmOn[alrmNum] ^ 4; //Toggle 
          break;
        case 6: // Sat
          Settings.alarmOn[alrmNum]=Settings.alarmOn[alrmNum] ^ 2; //Toggle 
          break;
        case 7: // Sun
          Settings.alarmOn[alrmNum]=Settings.alarmOn[alrmNum] ^ 1; //Toggle 
          break;
        case 8: // Exit
          subMenu[4+alrmNum]=0;
          subMenu[alrmNum]=4;//Move to next menu item
          cls();
          break;
      }
      playSFX(1);
      cls();
      break;
    case 4: // Set Alarm hours
      if (decrement) Settings.alarmHH[alrmNum]--; 
      else Settings.alarmHH[alrmNum]++;
      if (Settings.alarmHH[alrmNum]==255) Settings.alarmHH[alrmNum] = 23; // Negative number (byte) will be 255
      else if (Settings.alarmHH[alrmNum] > 23) Settings.alarmHH[alrmNum] = 0;      
      playSFX(6);  // tick
      break;
    case 5: // Set Alarm minutes  
      if (decrement) Settings.alarmMM[alrmNum]--; 
      else Settings.alarmMM[alrmNum]++;
      if (Settings.alarmMM[alrmNum] == 255) Settings.alarmMM[alrmNum] = 59; // Negative number (byte) will be 255
      else if (Settings.alarmMM[alrmNum] > 59) Settings.alarmMM[alrmNum] = 0;
      playSFX(6);  // tick
      break;
    case 6: // Set Alarm melody  
      if (decrement) Settings.alarmTone[alrmNum]--;
      else Settings.alarmTone[alrmNum]++;
      wave.stop();
      if (Settings.alarmTone[alrmNum] == 255) Settings.alarmTone[alrmNum] = ALARM_TONES; // Negative number (byte) will be 255
      else if (Settings.alarmTone[alrmNum] > ALARM_TONES) Settings.alarmTone[alrmNum] = 1;
      snprintf(myString,sizeof(myString), "ALRM%d.WAV",Settings.alarmTone[alrmNum]); // Make Alarm Filename
      playfile(myString);
      cls();
      break;
    case 7: // Alarm Progressive On/off
      if (decrement) Settings.alarmProgVol[alrmNum]-=5;  // subtract 5 seconds from progressive alarm interval
      else Settings.alarmProgVol[alrmNum]+=5;  // add 5 seconds to progressive alarm interval
      if (Settings.alarmProgVol[alrmNum]>60)  Settings.alarmProgVol[alrmNum]=0;  // wrap to Off at 60 seconds
      if (Settings.alarmProgVol[alrmNum] == 255)  Settings.alarmProgVol[alrmNum]=60;  // wrap from Off to 60
      cls();
      playSFX(1);
      break;
  }

}

// =======================================================================================
// ---- Show Alarm setting screen ----
// By: LensDigital
// =======================================================================================
void showAlarm(byte color){
//  if (!isSettingAlarm) return; // Exit if not setting alarm - caller checks now
  byte alrmNum;
  char myString[11];
//  alrmBlink(color);
  hhColor=color;  // set both to normal color first
  mmColor=color;
  if (blinking) {
    if (isSettingAlrmHH) {
      if (blinkDigit)  hhColor = BLACK;  // blink Hours
    }
    if (isSettingAlrmMM) {
      if (blinkDigit)  mmColor = BLACK;  // blink Minutes
    }
  }
  if (menuItem==1) alrmNum=0; // Decide which Alarm to show
  else alrmNum=1;
  switch (subMenu[alrmNum]) { 
    
    case 1: // Alarm On/off
      snprintf(myString,sizeof(myString),"Alrm%d",alrmNum+1);
      showText(1,8,myString,1,color);
      if (Settings.alarmOn[alrmNum] & 128) showText(10,0,"ON",1,hhColor); 
      else showText(10,0,"OFF",1,hhColor); 
      break;

    case 2:  // Alarm Frequency Setup (Off/Daily/Weekday/Custom)
      snprintf(myString,sizeof(myString),"Alrm%d",alrmNum+1);
      showText(1,8,myString,1,color);
      switch (Settings.alarmOn[alrmNum]) {
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
          if (Settings.alarmOn[alrmNum] & 64) showText(25,0,"Y",1,hhColor) ;
          else showText(25,0,"N",1,hhColor); 
          break;
        case 2: // Tue
          showText(0,0,"Tue:",1,color);
          if (Settings.alarmOn[alrmNum] & 32) showText(25,0,"Y",1,hhColor);
          else showText(25,0,"N",1,hhColor);
          break;
        case 3: // Wed
          showText(0,0,"Wed:",1,color);
          if (Settings.alarmOn[alrmNum] & 16) showText(25,0,"Y",1,hhColor);
          else showText(25,0,"N",1,hhColor);
          break;
        case 4: // Thu
          showText(0,0,"Thu:",1,color);
          if (Settings.alarmOn[alrmNum] & 8) showText(25,0,"Y",1,hhColor);
          else showText(25,0,"N",1,hhColor);
          break;
        case 5: // Fri
          showText(0,0,"Fri:",1,color);
          if (Settings.alarmOn[alrmNum] & 4) showText(25,0,"Y",1,hhColor);
          else showText(25,0,"N",1,hhColor);
          break;
        case 6: // Sat
          showText(0,0,"Sat:",1,color);
          if (Settings.alarmOn[alrmNum] & 2) showText(25,0,"Y",1,hhColor);
          else showText(25,0,"N",1,hhColor);
          break;
        case 7: // Sun
          showText(0,0,"Sun:",1,color);
          if (Settings.alarmOn[alrmNum] & 1) showText(25,0,"Y",1,hhColor);
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
      if (Settings.time12hr) {
        // == BEGIN 12 Hour Mode ====
        myhours=Settings.alarmHH[alrmNum]; // Get Alarm Hours 
        if (myhours==0) myhours=12; // Midnight
        else if (myhours >12) myhours=myhours-12;
//        if (alrmHH[alrmNum]<12)  plot (0,1,hhColor); // Show AM Dot
        if (Settings.alarmHH[alrmNum]>=12)  plot (0,1,hhColor); // Show PM Dot (wbp)
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
        snprintf(myString,sizeof(myString), "%02d",Settings.alarmHH[alrmNum]); // make 2 digit hours
        showText(1,0,myString,1,hhColor);
      }
      snprintf(myString,sizeof(myString), "%02d",Settings.alarmMM[alrmNum]);
      showText(12,0,":",1,color); // Show colon :
      showText(18,0,myString,1,mmColor);
      
      break;

    case 6: // Alarm Tone selection
      showText(1,0,"Tone:",1,color);
//      showText(0,8,"Alm",1,hhColor);
      snprintf(myString,sizeof(myString), "%02d",Settings.alarmTone[alrmNum]); // Make Alarm Filename
      showText(20,8,myString,1,hhColor); 
      break;

    case 7: // Alarm Progressive Volume On/off
      showText(1,0,"P.Vol:",1,color);
      if (Settings.alarmProgVol[alrmNum]>0) {
//        showText(10,8,"ON",1,hhColor);
        snprintf(myString,sizeof(myString), "%02d",Settings.alarmProgVol[alrmNum]);
        showText(3,8,myString,1,hhColor);
        showText(15,8,"sec",1,color);
      }
      else showText(10,8,"OFF",1,hhColor); 
      break;
    
  }
  
}


// =======================================================================================
// ---- Show  DST Menu (Daylight Savings Time) ----
// ---- by wbphelps
// =======================================================================================
void showDST(byte color) {
//  if (!isSettingDST) return; // Exit if not setting DST
  showText(6,0,"DST",1,color);
  hhColor = color; 
  if (blinkDigit)  hhColor = BLACK;  // blink setting value
  switch (Settings.DSTmode) {
    case 0: // DST off
    showText(6,8,"Off ",1,hhColor);
    break;
    case 1: // DST on
    showText(6,8,"On  ",1,hhColor);
    break;
    case 2: // DST Auto
    showText(6,8,"Auto",1,hhColor);
    break;
   }
}

// =======================================================================================
// ---- Show  TZ Hour or Minute ----
// ---- by wbphelps
// =======================================================================================
void showTZ(byte color) {
  char myString[11];
  showText(4,0,"Zone",1,color);
  hhColor = color;
  if (isSettingTZMinute)
    snprintf(myString,sizeof(myString), "Mn %d ",Settings.TZ_minute);
  else
    snprintf(myString,sizeof(myString), "Hr%+0.02d ",Settings.TZ_hour);
  if (blinkDigit)  hhColor = BLACK;  // blink setting value
  showText(0,8,myString,1,hhColor);
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
      playSFX(6); // tick
      break;
    case 2: // Set minutes
      if (decrement) minutes--;
      else minutes++;
      if (minutes == 255) minutes = 59;
      else if (minutes > 59) minutes = 0;
      seconds=0; // Reset seconds to 0 with each minute change
      playSFX(6); // tick
      break;
    case 3: // Set months
       if (decrement) months--;
       else months++;
       if (months == 0) months=12;
      else if (months > 12) months=1;
      playSFX(6); // tick
      break;
    case 4: // Set Days
      if (decrement) days--;
      else  days++;
      if (days == 255) days = 31;
      else if (days > mdays)  days = 1;
      playSFX(6); // tick
      break;
    case 5: // Set Years 
      if (decrement) years--;
      else years++;
      if (years < 10 ) years = 40;
      else if (years > 40) years = 10; // Default to 2010
      playSFX(6); // tick
      break;
    case 6: // DST
      playSFX(1);
      Settings.DSTmode = ++Settings.DSTmode % 3;  // off, on, Auto
      if (Settings.DSTmode == 1) { // on
        hours++;  // spring ahead
        Settings.DSToffset = 1;  // remember offset
      }
      else { // off or Auto
        if (Settings.DSToffset) { // was it set?
          hours--;  // fall back
          Settings.DSToffset = 0;
        }
      }
      if (hours == 255)  hours = 23;  // handle wrap
      else if (hours > 23)  hours = 0;
//      g_DST_updated = false; // re-init & re-calc DST for today
      break;
    case 7: // TZ Hour
      if (decrement) Settings.TZ_hour--;
      else Settings.TZ_hour++;
      if (Settings.TZ_hour>13)  Settings.TZ_hour = -12; // wrap
      if (Settings.TZ_hour<-12)  Settings.TZ_hour = 13; // wrap
      break;
    case 8: // TZ Minute
      if (decrement) Settings.TZ_minute-=15;
      else Settings.TZ_minute+=15;
      if (Settings.TZ_minute>45)  Settings.TZ_minute = 0; // wrap
      if (Settings.TZ_minute<0)  Settings.TZ_minute = 45; // wrap
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
  char string1[10];
 //putstring_nl ("Setting System");
  if (subMenu[3] && (subMenu[3]!=7))  playSFX(1); // Don't play sound if not setting anything, i.e. submenu=0
  switch (subMenu[3]) {
    case 1: // 12/24 Hour Mode
        if (!Settings.time12hr) Settings.time12hr=true; 
        else Settings.time12hr=false ; 
        break;
     case 2: // Set Display Brightness
        cls();
        if (decrement) Settings.brightness--;
        else Settings.brightness++;
        if (Settings.brightness == 255) Settings.brightness = 5;
        else if (Settings.brightness > 5) Settings.brightness = AUTO_BRIGHTNESS_ON;  // 0 if photocell present, else 1
        prevBrightness=0; // force an update
        if (Settings.brightness==0) autoBrightness();
        else
          setBrightness(Settings.brightness);
        break;
     case 3: // Set Clock Color
       cls();
       if (Settings.autoColor)
         Settings.autoColor = false;
       else if (decrement) {
         if (Settings.clockColor>1)
           Settings.clockColor--;
         else {
           Settings.clockColor = 3;
           Settings.autoColor = true;
         }
         clockColor = Settings.clockColor;  // set working copy of clockColor
       }
       else {
         if (Settings.clockColor<3)
           Settings.clockColor++;
         else {
           Settings.clockColor = 1;
           Settings.autoColor = true;
         }
         clockColor = Settings.clockColor;  // set working copy of clockColor
       }
       break;
     case 4: // Set Clock Font (0-5)
       cls();
       if (decrement) Settings.clockFont--;
       else Settings.clockFont++;
       if (Settings.clockFont ==255) Settings.clockFont=4;
       else if (Settings.clockFont>4) Settings.clockFont=0;
       break;
     case 5: // Set sFX on/off
       cls();
       if (Settings.sFX) Settings.sFX=false;
       else  Settings.sFX=true;
       break;
     case 6: // Temperature Units
       cls();
       if (Settings.tempUnit) Settings.tempUnit=false;
       else  Settings.tempUnit=true;
       break;
      case 7: // Sound Volume - 0 is lowest, 7 is loudest
        cls();
        if (decrement) Settings.soundVol--;
        else Settings.soundVol++;
        if (Settings.soundVol == 255)  Settings.soundVol=MAX_VOLUME; // wrap
        else if (Settings.soundVol > MAX_VOLUME)  Settings.soundVol=0;
//        playSFX(1);  // play tone AFTER changing volume!
        snprintf(string1,10, "%d.WAV",Settings.soundVol+1); // Make Filename
        playfile(string1);  // play it
        break;
      case 8: // cursor blink
        cls(); 
        if (Settings.cursorBlink) Settings.cursorBlink=false; // Turn off blinking cursor
        else Settings.cursorBlink=true; // Turn on blinking cursor
        //playSFX(1);
      break;
      case 9: // Startup on/off
        cls();
//        if (Settings.doStartup) Settings.doStartup=false;
//        else Settings.doStartup=true;
        Settings.startup++;
        if (Settings.startup>2)  Settings.startup=0;
        //playSFX(1);
        break;
      case 10: // GPS
        cls();
#ifdef GPS_PRESENT  // If GPS Hardware defined globally
        if (Settings.GPSenabled)  Settings.GPSenabled=false; // Toggle GPS
        else {
          Settings.GPSenabled=true; // Turn on GPS
          gpsInit(9600);  // init GPS & Serial port for 9600 BPS
        }
#else
        playSFX(5);
        Settings.GPSenabled=false;
#endif
      break;
      case 11: // IR
        cls(); 
#ifdef IR_PRESENT  // If IR Hardware defined globally
        if (Settings.IRenabled) Settings.IRenabled=false; // Toggle IR setting
        else {
          Settings.IRenabled=true; // Turn on IR
          irrecv.enableIRIn(); // Start the IR receiver. Comment out if IR not present
        }
#else
        playSFX(5);
        Settings.IRenabled=false;
#endif
      break;
      case 12: // RFM
        cls();
#ifdef RFM12B_PRESENT  // If chip phisically present
        if (Settings.RadioEnabled)  Settings.RadioEnabled=false;  // toggle Radio
        else {
          Settings.RadioEnabled=true;
          delay (15);
        }
#else
        playSFX(5);
        Settings.RadioEnabled=false;
#endif
      break;
  }

}

// =======================================================================================
// ---- Show System setting screen ----
// By: LensDigital
// =======================================================================================
void showSys(){
//  if (!isSettingSys) return; // Exit if not setting system
  byte color=clockColor;
  hhColor = color;
  if (blinkDigit && !isAdjusting)  hhColor = BLACK;  // blink setting value
  switch (subMenu[3]){ 
    case 1: // We are setting 12/24 Hour mode
      showText(1,0,"Mode:",1,color);
      if (Settings.time12hr) showText(1,8,"12 HR ",1,hhColor);
      else showText(1,8,"24 HR",1,hhColor);
      break;
    case 2:  // Adjust Brightness Level
      showText(1,0,"Light",1,color);
      //Serial.println (brightness);
      switch (Settings.brightness) {
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
      if (blinkDigit) {
        if (Settings.autoColor)
          showText(1,8,"Auto",1,ORANGE);
        else switch ( Settings.clockColor) {
         case RED:  
           showText(8,8,"Red",1,RED);
           break;  
         case GREEN:  
           showText(1,8,"Green",1,GREEN);
           break;
         case ORANGE:  
           showText(1,8,"Yello",1,ORANGE);
           break;
        }
      }
      else showText(1,8,"     ",1,BLACK);  // blink the setting
    break;
    case 4: // Set Clock Font
      char myString[8];
      showText(1,0,"Font:",1,color);
      snprintf(myString,sizeof(myString), "Font%d",Settings.clockFont); // Show current font number
      showText(1,8,myString,1,hhColor);
    break;
    case 5: // Enable/Disable menu SFX
      showText(2,0,"Menu",1,color);
      showText(0,8,"sFX",1,color);
      if (Settings.sFX) showText(18,8,"ON",3,hhColor);
      else showText(18,8,"OFF",3,hhColor);
    break;
    case 6: // Temperature C or F
      showText(2,0,"Temp",1,color);
      if (Settings.tempUnit) showText(10,8,"F",1,hhColor);
      else  showText(10,8,"C",1,hhColor);
      ht1632_putchar(18,8,127,color); // Show Degree Sign
      break;
    case 7: // Sound Volume
      byte barColor; // Color of volume bar
      showText(0,0,"Sound",1,color);
      showText(2,9,"VOL",1,color);
      for (int x=0;x<=Settings.soundVol;x++)  // x value, 0 to volume level
        for (int y=0;y<=x;y++)  // y value
          plot ( (x+23),(15-y), color);  // plot volume bar graph
      break; 
    case 8: // blink
      showText(2,0,"Blink",1,color);
      if (Settings.cursorBlink) showText(10,8,"ON",1,hhColor); 
      else showText(10,8,"OFF",1,hhColor);
    break;
    case 9: // Startup on/off
      showText(0,0,"Startup",3,color);
      switch ( Settings.startup) {
        case 0:
          showText(5,8,"OFF",1,hhColor); 
        break;
        case 1:
          showText(5,8,"ON",1,hhColor); 
        break;
        case 2:
          showText(2,8,"Quiet",1,hhColor); 
        break;
      }
    break;
    case 10: // GPS Receiver
      showText(2,0,"GPS",1,color); 
#ifdef GPS_PRESENT  // If GPS Hardware defined globally
      if (Settings.GPSenabled) showText(10,8,"ON",1,hhColor); 
      else showText(10,8,"OFF",1,hhColor);
#else
      showText(10,8,"N/A",1,hhColor);
#endif
    break;
    case 11: // IR Receiver
      showText(2,0,"IR",1,color); 
#ifdef IR_PRESENT  // If IR Hardware defined globally
      if (Settings.IRenabled) showText(10,8,"ON",1,hhColor); 
      else showText(10,8,"OFF",1,hhColor);
#else
      showText(10,8,"N/A",1,hhColor);
#endif
    break;
    case 12: // RF Module
      showText(2,0,"RFM12",1,color); 
#ifdef RFM12B_PRESENT  // If IR Hardware defined globally
      if (Settings.RadioEnabled) showText(10,8,"ON",1,hhColor); 
      else showText(10,8,"OFF",1,hhColor);  
#else
      showText(10,8,"N/A",1,hhColor);
#endif
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
         //putstring_nl ("Enter scroll Submenu");
         break;
       case 1: // Frequency
         if (decrement) Settings.infoFreq--;
         else Settings.infoFreq++;
         if (Settings.infoFreq == 0) Settings.infoFreq=9;
         else if (Settings.infoFreq > 9) Settings.infoFreq=0;
         cls();
       break;
       case 2: // Scroll Date
//         putstring_nl ("Scroll Date");
         Settings.infoOptions = Settings.infoOptions ^ IO_Date; //Toggle
        break;
       case 3: // Scroll Internal temp
//         putstring_nl ("Scroll Temp");
         Settings.infoOptions = Settings.infoOptions ^ IO_InTemp; //Toggle
         break;
       case 4: // Scroll Internal humidity
#if defined DHT22
         Settings.infoOptions = Settings.infoOptions ^ IO_InHum; //Toggle
#else
         playSFX(5);  // no radio
#endif
         break;
       case 5: // Scroll External Temp
#ifdef RFM12B_PRESENT
         Settings.infoOptions = Settings.infoOptions ^ IO_OutTemp; //Toggle
#else
         playSFX(5);  // no radio
#endif
         break;
       case 6: // Scroll  Alarm
         Settings.infoOptions = Settings.infoOptions ^ IO_Alarms; //Toggle
         break;
       case 7: // Scroll  Sensor Data
#ifdef RFM12B_PRESENT
         Settings.infoOptions = Settings.infoOptions ^ IO_Sensor; //Toggle
#else
         playSFX(5);
#endif
         break;
       case 8: // Scroll  Humidity data
#ifdef RFM12B_PRESENT
         Settings.infoOptions = Settings.infoOptions ^ IO_OutHum; //Toggle
#else
         playSFX(5);
#endif
         break;
       case 9: // Exit
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
         Settings.sayOptions = Settings.sayOptions ^ SO_Time; //Toggle
         break;
      case 2: // Say Date
         Settings.sayOptions = Settings.sayOptions ^ SO_Date; //Toggle
         break;
      case 3: // Say Internal Temp
         Settings.sayOptions = Settings.sayOptions ^ SO_InTemp; //Toggle
         break;
      case 4: // Say Internal Humidity
         Settings.sayOptions = Settings.sayOptions ^ SO_InHum; //Toggle
         break;
      case 5: // Say External Temp
         //Serial.println ("Saving say Ext Temp");
#ifdef RFM12B_PRESENT
         Settings.sayOptions = Settings.sayOptions ^ SO_OutTemp; //Toggle
#else
         playSFX(5);
#endif
         break;
      case 6: // Say External Humidity
#ifdef RFM12B_PRESENT
         Settings.sayOptions = Settings.sayOptions ^ SO_OutHum; //Toggle
#else
         playSFX(5);
#endif
         break;
      case 7: // Say  Alarm
         Settings.sayOptions = Settings.sayOptions ^ SO_Alarms; //Toggle
         break;
      case 8: // Exit
         subMenu[8]=0;
         break;
     }
  break;
  
  case 3: // temperature offset
     if (decrement) Settings.tempOffset--;
     else Settings.tempOffset++;
     if (Settings.tempOffset == 255) Settings.tempOffset=9;
     else if (Settings.tempOffset>9) Settings.tempOffset=0;
     cls();
   break;
   
  case 4: // pcell display/set
     switch (subMenu[9]) {
       case 0: // Enter Menu
         cls();
         subMenu[9]=1;
         isAdjusting=false;  // not adjusting anything, don't get trapped by Set button becoming Increment
         //putstring_nl ("Enter pcell Submenu");
         break;
       case 1:  // just displaying pcell reading, nothing to adjust
         isAdjusting=false;  // not adjusting anything, don't get trapped by Set button becoming Increment
         break;
       case 2:
         if (decrement) {
           if (Settings.photoCellMin>10) Settings.photoCellMin -=10;
           else Settings.photoCellMin = 100;  // wrap around
         }
         else {
           if (Settings.photoCellMin < 100) Settings.photoCellMin += 10;
           else Settings.photoCellMin = 0;  // wrap around
         }
         break;
       case 3:
         if (decrement) { 
           if (Settings.photoCellMax>100) Settings.photoCellMax -=50;
           else if (Settings.photoCellMax>50) Settings.photoCellMax -=10;
           else Settings.photoCellMax = 500;  // wrap around to 500
         }
         else {  // increment
           if (Settings.photoCellMax < 100) Settings.photoCellMax += 10;
           else if (Settings.photoCellMax < 500) Settings.photoCellMax += 50;
           else Settings.photoCellMax = 50;  // wrap around to 50
         }
         break;
       case 4:
         subMenu[9]=0;  // exit from pcell submenu
         break;
     }
   break;
  
  case 5: // snooze time
     if (decrement) Settings.alarmSnoozeTime--;
     else Settings.alarmSnoozeTime++;
     if (Settings.alarmSnoozeTime > 15)  Settings.alarmSnoozeTime=1;  // wrap at 15
     if (Settings.alarmSnoozeTime < 1)  Settings.alarmSnoozeTime=15;  // wrap at 1
     cls();
   break;
 
 }
 
}

// =======================================================================================
// ---- Show User Options setting screen ----
// By: LensDigital
// =======================================================================================
void showOpt(){
//  if (!isSettingOptions) return; // Exit if not setting options
  byte color=clockColor;
  char myString[2]; 
  int blinkDigDuration =500;
  int pcell;
  hhColor = color;  // assume normal color
  if (blinkDigit && !isAdjusting)  hhColor = BLACK;  // blink setting value
  switch (subMenu[6]){ 
   case 1: // InfoDisplay
      if (subMenu[7]==0) { // Will skip showing this text if we are deeper in submenu
       showText(0,0,"Info",1,color);
       showText(0,8,"Displ",1,color);
       }
     switch (subMenu[7]){  
     case 1: // Frequency
       showText(1,0,"Show:",1,color);
       if (Settings.infoFreq==0) showText(1,8,"Never",1,hhColor);
       else {
        showText(0,8,"Every:",3,color);
        snprintf(myString,sizeof(myString), "%d",Settings.infoFreq); // Make string
        showText(24,8,myString,3,hhColor);
       }
      break;
      case 2: // Scroll Date
       showText(0,0,"Displ",1,color);
       showText(0,8,"Date",1,color);
       if (Settings.infoOptions & IO_Date) showText(25,8,"Y",1,hhColor) ;
       else showText(25,8,"N",1,hhColor); 
      break;
      case 3: // Scroll Inside Temp
       showText(0,0,"Displ",1,color);
       showText(0,8,"InTemp",3,color);
       if (Settings.infoOptions & IO_InTemp) showText(25,8,"Y",1,hhColor) ;
       else showText(25,8,"N",1,hhColor);
      break;
      case 4: // Scroll Inside Humidity
#if defined DHT22 
       showText(0,0,"Displ",1,color);
       showText(0,8,"InHum",3,color);
       if (Settings.infoOptions & IO_InHum) showText(25,8,"Y",1,hhColor) ;
       else showText(25,8,"N",1,hhColor);
#else
       subMenu[7]++;  // skip inside humidity
#endif
      break;
      case 5: // Scroll Outside Temp
       if (!Settings.RadioEnabled) { // Skip if Receiver is not enabled
         subMenu[7]++;
         break;
       }
       showText(0,0,"Displ",1,color);
       showText(0,8,"ExTemp",3,color);
       if (Settings.infoOptions & IO_OutTemp) showText(25,8,"Y",1,hhColor) ;
       else showText(25,8,"N",1,hhColor);
      break;
      case 6: // Scroll Alarm
       showText(0,0,"Displ",1,color);
       showText(0,8,"Alarm",3,color);
       if (Settings.infoOptions & IO_Alarms) showText(25,8,"Y",1,hhColor) ;
       else showText(25,8,"N",1,hhColor);
      break;
      case 7: // Scroll receiver stats
       if (!Settings.RadioEnabled) { // Skip if Receiver is not enabled
         subMenu[7]++;
         break;
       }
       showText(0,0,"Displ",1,color);
       showText(0,8,"RFStat",3,color);
       if (Settings.infoOptions & IO_Sensor) showText(25,8,"Y",1,hhColor) ;
       else showText(25,8,"N",1,hhColor);
      break;
      case 8: // Scroll humidity
       if (!Settings.RadioEnabled) { // Skip if Receiver is not enabled
         subMenu[7]++;
         break;
        }
        showText(0,0,"Displ",1,color);
        showText(0,8,"Humid",3,color);
        if (Settings.infoOptions & IO_OutHum) showText(25,8,"Y",1,hhColor) ;
        else showText(25,8,"N",1,hhColor);
      break;
      case 9: // Exit
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
       if (Settings.sayOptions & SO_Time) showText(25,8,"Y",1,hhColor) ;
       else showText(25,8,"N",1,hhColor); 
     break;
     case 2: // Say Date
       showText(0,0,"Say",1,color);
       showText(0,8,"Date",1,color);
       if (Settings.sayOptions & SO_Date) showText(25,8,"Y",1,hhColor) ;
       else showText(25,8,"N",1,hhColor); 
     break;
     case 3: // Say Inside Temperature
       showText(0,0,"Say",1,color);
       showText(0,8,"InTemp",3,color);
       if (Settings.sayOptions & SO_InTemp) showText(25,8,"Y",1,hhColor) ;
       else showText(25,8,"N",1,hhColor); 
     break;
     case 4: // Say Inside Humidity
       showText(0,0,"Say",1,color);
       showText(0,8,"InHum",3,color);
       if (Settings.sayOptions & SO_InHum) showText(25,8,"Y",1,hhColor) ;
       else showText(25,8,"N",1,hhColor); 
     break;
     case 5: // Say Outside Temperature
       if (!Settings.RadioEnabled) { // Skip if Receiver is not enabled
         subMenu[8]++;
         break;
       }
       showText(0,0,"Say",1,color);
       showText(0,8,"ExTemp",3,color);
       if (Settings.sayOptions & SO_OutTemp) showText(25,8,"Y",1,hhColor) ;
       else showText(25,8,"N",1,hhColor); 
       break;
     case 6: // Say Humidity
       if (!Settings.RadioEnabled) { // Skip if Receiver is not enabled
         subMenu[8]++;
         break;
       }
       showText(0,0,"Say",1,color);
       showText(0,8,"Humid",3,color);
       if (Settings.sayOptions & SO_OutHum) showText(25,8,"Y",1,hhColor) ;
       else showText(25,8,"N",1,hhColor); 
       break;
     case 7: // Say Alarm
       showText(0,0,"Say",1,color);
       showText(0,8,"Alrm",1,color);
       if (Settings.sayOptions & SO_Alarms) showText(25,8,"Y",1,hhColor) ;
       else showText(25,8,"N",1,hhColor); 
       break;
     case 8: // Exit
       showText(0,0,"Exit",1,color);
       break;
     }
   break;

   case 3: // Temperature offset
     showText(0,0,"Temp",1,color);
     showText(0,8,"Offs:-",3,color);
     snprintf(myString,sizeof(myString), "%d",Settings.tempOffset); // Make string
     showText(25,8,myString,1,hhColor);
   break;

   case 4: // Photocell sub menu
     if (subMenu[9]==0) { // Will skip showing this text if we are deeper in submenu
       showText(0,0,"Photo",1,color);
       showText(4,8,"cell",1,color);
     }
     switch (subMenu[9]) {
      case 1: // show pcell reading
       if (Settings.brightness)  // if brightness not auto,
         photoCell = (photoCell + analogRead(photoCellPin))/2;  // average of 2 readings
       else
         autoBrightness();  // read photocell & set brightness
       showText(0,0,"Pcell",1,color);
       snprintf(myString,4, "%3d",photoCell); // Make string
       showText(10,8,myString,1,color);  // display pcell reading (no blink)
//       blinkTime = millis();  // don't blink until you see the whites of their eyes
       hhColor = color;
       lastButtonTime = millis()-500;  // don't exit either
      break;
      case 2: // Brightness Low
       showText(0,0,"BrtLo",1,color);
       snprintf(myString,4, "%3d",Settings.photoCellMin); // Make string
       showText(10,8,myString,1,hhColor);
      break;
      case 3: // Brightness High
       showText(0,0,"BrtHi",1,color);
       snprintf(myString,4, "%3d",Settings.photoCellMax); // Make string
       showText(10,8,myString,1,hhColor);
      break;
      case 4: // Exit
       showText(0,0,"Exit",1,color);
      break;
     }
   break;

   case 5: // alarm snooze time
     showText(0,0,"Snooze",1,color);
     showText(0,8,"Time:",3,color);
     snprintf(myString,3, "%02d",Settings.alarmSnoozeTime); // Make string
     showText(20,8,myString,1,hhColor);
   break;

  }
}

//// =======================================================================================
//// ---- Check Incr button to show Quick Menu ----
//// ---- Press and quickly release to go thru one item at a time
//// ---- Hold button for more than a second to autoamtically go thru all items.
//// ---- by LensDigital
//// =======================================================================================
//void quickMenu() {
//  // check MENU/INCR button;
//    currStatusInc=digitalRead(INC_BUTTON_PIN);
//    if (currStatusInc!=lastStatusInc) {// Change Occured 
//       lastStatusInc=currStatusInc;
//       if (isInMenu) return; //Make sure we are not in settings menu
//       quickDisplay(true); // Process Quick Menu functions
//    }
//  if (isInQMenu)
//    if ( (digitalRead(SET_BUTTON_PIN) == HIGH) || (digitalRead(MENU_BUTTON_PIN) == HIGH) ){
//       lastButtonTime = 0; // Exit Quick Menu if any button other than INC was pres
//      //processSetButton(); // Only to control alarm
//    }
//   // display the menu option for 10 sec after menu button was pressed;
//  if ((lastButtonTime > 0) && (millis() - lastButtonTime < 10000))  // wbp
//    return;
//  
//  // Finished with menus, return to normal operations
//  if (isInQMenu) {
//    putstring_nl ("Exiting QMenu");
//    cls ();
//    okClock=true; // Enable Big Clock display
//    isSettingDate=false;
//    isInQMenu=false;
//    mbutState=1; // Reset Menu Button State
//    isIncrementing = false;
//    blinking = false;
//    isSettingAlarm=false;
//    isSettingDST=false; // wbp
//    menuItem=0; 
//    subMenu[0]=0;
//    subMenu[1]=0;
//    subMenu[9]=0; // wbp
//    mbutState=1;
//    cls();
//  }
//}

// =======================================================================================
// ---- Play menu sounds (sFX) ----
// ---- Pass 1 for menu naviagtion, 2 chnging item , 3 for exit, 4 for ?, 5 for error
// ---- by LensDigital
// =======================================================================================
void playSFX(byte item){
  if (!Settings.sFX) return; // Global Sound Effects are off
#ifdef RFM12B_PRESENT
  turnOffRadio(); // Disable RF12B
#endif
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
    playfile("MENU4.WAV");
    break;    
  case 5: 
    playfile("ERR1.WAV");
    break;    
  case 6: 
    playfile("TICK.WAV");
    break;    
  }
  //radio.Wakeup(); // Disable RF12B
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
         if (subMenu[7] > 9) subMenu[7]=1; // Goes back to first item of this submenu 
       }
     }
     else if (subMenu[6]==2) { // Do not increment this submenu (we are in Talking Options menu)
       //putstring_nl ("In subMenu");
       if (subMenu[8] == 0) subMenu[6]++; // Go to next item, we are not enterying this submenu tree
       else {
         subMenu[8]++;
         if (subMenu[8] > 8) subMenu[8]=1; // Goes back to first item of this submenu 
       }
     } 
// added for pcell sub menu
     else if (subMenu[6]==4) { // Do not increment this submenu (we are in Pcell Options menu)
       //putstring_nl ("In subMenu");
       if (subMenu[9] == 0) subMenu[6]++; // Go to next item, we are not entering this submenu tree
       else {
         subMenu[9]++;
         if (subMenu[9] > 4) subMenu[9]=1; // Goes back to first item of this submenu 
       }
     } 

     else subMenu[6]++; // Increment button press count
     if (subMenu[6] > MAX_OPTIONS) subMenu[6]=1; // Goes back to first menu item
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
//  if (subMenu[2] > 7) subMenu[2]=1; // Goes back to first menu item
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
      isSettingTZ = false;
      isSettingTZMinute = false;
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
    case 7: // TZ hour
      isSettingDST = false;
      isSettingTZ = true;
      isSettingTZMinute = false;
      break;
    case 8: // TZ minute
//      isSettingTZHour = false;
      isSettingTZMinute = true;
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
  if (!(Settings.alarmOn[alrmNum] & 128) && subMenu[alrmNum]==2)  subMenu[alrmNum]=4; // ALarm is off so we need to skip 4nd menu
  if (!Settings.alarmCustom[alrmNum] && subMenu[alrmNum]==3) subMenu[alrmNum]=4; // Custom Alarm is not set so skip to 4th menu
  switch (subMenu[alrmNum]) {
    case 1: // Set Alarm on Off
//      cls();
      isSettingDate = false;
      isSettingAlarm = true;
      isSettingAlrmHH = true;
      break;
   case 2: // SET: Alarm Dalily/Weekday/Custom
      //putstring_nl ("SET: Alarm OFF/Dalily/Weekday/Custom");
//      cls();
      isSettingDate = false;
      isSettingAlarm = true;
      isSettingAlrmHH = true;
      break;
    case 3: // Set Alarm Custom schedule
      if (Settings.alarmCustom[alrmNum]) {
        //putstring_nl ("SET: Alarm1. Custom Schedule");
        subMenu[4+alrmNum]++;
        if (subMenu[4+alrmNum] > 8) subMenu[4+alrmNum]=1; // Goes back to first menu item (Monday)
      }
      isSettingAlrmHH = true;
      isSettingAlrmMM = false;
      break;
    case 4: // Set Alarm Hrs
      //putstring_nl ("SET: Alarm HRS");
//      cls();
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

