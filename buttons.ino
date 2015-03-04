// =======================================================================================
// ---- Check button and set state ----
// by wbp
// =======================================================================================
static byte buttonState[3] = {0,0,0};
static unsigned int buttonHoldTime = 20;  // how long button must be held
// note: only supports one button down at a time, Menu takes priority
// button states:
#define BS_OFF       0  // 0 - not down
#define BS_DEBOUNCE  1  // 1 - down, debouncing
#define BS_HOLDING   2  // 2 - held, waiting for repeat
#define BS_RELEASED  3  // 3 - pressed & released, no hold
#define BS_PRESSED   4  // 4 - pressed, still down, not repeating
#define BS_REPEATING 5  // 5 - held, repeat
void checkButton(byte btn, byte idx) {
  if (digitalRead(btn) == HIGH) {
    if (buttonState[idx] == 0) {  // was the button just pressed?
      lastButtonTime = millis();  // start button debounce timer (wbp)
      buttonState[idx] = BS_DEBOUNCE;  // button down, waiting for debounce
      buttonHoldTime = BUTTON_DEBOUNCE_TIME;  // first hold is just debounce
//      Serial.println("b1 1");
    }
    else if (buttonState[idx] >= BS_PRESSED) {  // did we signal that button was pressed (4) or repeating (5)?
      buttonState[idx] = BS_HOLDING;  // do that once per interval
//      Serial.println("b1 2");
    }
    if ((millis() - lastButtonTime) >= buttonHoldTime) {  // has debounce or hold time been reached?
      lastButtonTime = millis();  // restart button timer
      if (buttonState[idx] == BS_DEBOUNCE) {  // button just pressed
        buttonState[idx] = BS_PRESSED;  // signal button pressed once
//        Serial.println("b1 4");
        buttonHoldTime = BUTTON_HOLD_TIME;  // longer hold before repeat
      }
      else if (buttonState[idx] == BS_HOLDING) { // button holding for repeat
        buttonState[idx] = BS_REPEATING;  // signal button repeat
//        Serial.println("b1 5");
        buttonHoldTime = BUTTON_REPEAT_TIME;  // hold time = repeat time
      }
    }
  }
  else {  // button not down
    if (buttonState[idx] == BS_HOLDING) {  // was it held for at least the debounce time?
      buttonState[idx] = BS_RELEASED;  // button released
//      Serial.println("b1 3");
    }
    else {
      buttonState[idx] = BS_OFF;  // button not pressed
    }
  }
}

// =======================================================================================
// ---- Check button input and act upon it ----
// Adopted from Florinc by Bratan
// =======================================================================================
void buttonProc(){
  // check a few things first
  showAlarm(clockColor); // Display Alarm setting screen
  if (!isInQMenu) mainDate(clockColor); // Show date setting screen, but only if we are not in QMenu
  showSys();
  showOpt();
  showDST(clockColor);

  // check the buttons. Menu button takes priority, then Set, then Inc
  checkButton(MENU_BUTTON_PIN,0);  // check Menu button
  if (buttonState[0]==0) {
     checkButton(SET_BUTTON_PIN,1);  // check Set button
     if (buttonState[1]==0)
       checkButton(INC_BUTTON_PIN,2);  // check Inc button
  }

  // check MENU button;
  if (buttonState[0]>=BS_RELEASED)  processMenuButton(buttonState[0]);
  // Check Set button
  if (buttonState[1]>BS_RELEASED)  processSetButton(buttonState[1]);

  // If in Menu, check INCR button, check for Menu timeout
  if (isInMenu) {
    if (buttonState[2]>BS_RELEASED)  processIncButton(buttonState[2]);
    // display the menu option for 5 seconds after menu button was pressed;
    // start blinking again if no button pushed for 1 second
    if ((lastButtonTime > 0) && (millis() - lastButtonTime < 5000)) {  // wbp
      if ( millis() - lastButtonTime > 1000 )  { // Start blinking if buttons not touched for a second
        isIncrementing = false;
        if (!isInQMenu) 
          startBlinking();
      }
      return;
    }
    // return the main mode if no button was pressed for 5 seconds;
    // Exit and reinitialize
    exitMenus();
  }  // if (isInMenu)
  else {  // not in Menu
    if (buttonState[2]==BS_REPEATING)  quickDisplay(true);  // button held, do long Quick menu
    else if (lastButtonTime && buttonState[2]==BS_RELEASED)  quickDisplay(false);  // button released, do short Quick Menu
    if (isInQMenu) {
//      if ( (buttonState[0]>BS_DEBOUNCE) || (buttonState[1]>BS_DEBOUNCE) ) {
//         lastButtonTime = 0; // Exit Quick Menu if any button other than INC was pres
//      //processSetButton(); // Only to control alarm
//      }
//    
      // display the menu option for 10 sec after menu button was pressed;
      if ((lastButtonTime > 0) && (millis() - lastButtonTime < 10000))  // wbp
        return;
  
      // Finished with menus, return to normal operations
      //putstring_nl ("Exiting QMenu");
      exitMenus();
    }
  }
  
}

// =======================================================================================
// ---- Process Menu Button ----
// Adopted from Florinc by Bratan 
// =======================================================================================
void processMenuButton(byte buttonState)
{
//  if (isInQMenu) return; // We are in quick menu, so don't show settings menu
  if (isInQMenu) {  // exit quick menu if active
    isInQMenu=false;
  }
//  Serial.print("Menu: "); Serial.println(buttonState);
    
 // ====  Alarm control  ====
  if ( (alarmState[0] >= AS_SOUNDING) || (alarmState[1] >= AS_SOUNDING) ) {  // sounding alarm or snoozing?
    if (buttonState < BS_REPEATING)  return;  // If button not held, ignore it
    resetAlrm(0);  // reset both alarms
    resetAlrm(1);
    return;  // all done
  }

  // ===  Skip Alarm  ===
  // not already in the menu, and either alarm is due in less than 2 hours, and Menu held down, set alarm skip flag
  if (!isInMenu && buttonState>BS_RELEASED) {  // if not in menu, check to see if either alarm is pending
    unsigned int alarmt1 = timeToNextAlarm(0);
    unsigned int alarmt2 = timeToNextAlarm(1);
//    Serial.print("at1 = "); Serial.println(alarmt1);
//    Serial.print("at2 = "); Serial.println(alarmt2);
    if ( alarmt1<=60 || alarmt2<60) {  // is there an alarm pending?
      // need to add check for skip already set, so we can toggle it back off...
      if (buttonState == BS_REPEATING) {  // hold Menu/Reset button to skip pending alarm
        if (alarmt1<120)  skipAlrm(0);
        if (alarmt2<120)  skipAlrm(1);
      }
    return;
    }
  }
  else
    if (buttonState==BS_RELEASED)  return;  // ignore button release if already in menu
  
  //putstring_nl ("Is In Menu Button Proc");
  timeSettings();  // reset Settings timer
  last_ms=millis(); // Create Offset for keeping track of seconds
  isInMenu = true;
  startBlinking();
  isSettingAlarm = false;
  isSettingTime = false;
  isSettingDate = false;
  menuItem++; // Increment button press count
  if (menuItem > MAX_MENUS) menuItem=1; // Goes back to first menu item
  showMenu();
  
}


// =======================================================================================
// ---- Process Set Button ----
// Adopted from Florinc by Bratan (added date setting)
// =======================================================================================
void processSetButton(byte buttonState)
{
//  Serial.print("Set: "); Serial.println(buttonState);
  
  // If Alarms sound it will stop them and snooze
  snoozeProc(0);
  snoozeProc(1);
 
  if (!isInMenu) return; // failsafe
  timeSettings();  // reset Settings timer
  //isInMenu    = true;
  playSFX(2);
  switch (menuItem) {
    case 1: // Begin Set Alarm 1
    //putstring_nl ("SET: Alarm1");
    butSetAlarm(0);
    break; // End set Alarm 1
    case 2: // Begin Set Alarm 2
    //putstring_nl ("SET: Alarm2");
    butSetAlarm(1);
    break; // End set Alarm
    case 3: // Begin Set Time and Date
        butSetClock();
    break;

    case 4: // Begin System Settings
        cls();
        isSettingAlarm = false;
        isSettingTime   = false;
        isSettingSys=true;
        subMenu[3]++; // Increment button press count
        if (subMenu[3] > MAX_SYSSETTINGS) subMenu[3]=1; // Goes back to first menu item
    break;
    case 5: // User options
        cls();
        userMenu();
    break;
    }
}


// =======================================================================================
// ---- Process Increment button ----
// Adopted from Florinc by Bratan
// =======================================================================================
void processIncButton(byte buttonState)
{  
//  Serial.print("Inc: "); Serial.println(buttonState);
  timeSettings();  // reset Settings timer
  isIncrementing = true;
  stopBlinking();  
  switch (menuItem) {
    case 1: // Setting Alarm 1
      setAlarm(0);
      break;
    case 2: // Setting Alarm 2
      setAlarm(1);
      break;  
    case 3: // Setting Time and Date
      setTimeDate();
      break;
    case 4: // System Settings
      sysSetting();
      break;
    case 5: // User Options settings
      optSetting();
    break;
  }
}


void exitMenus() {
    // Exit and reinitialize
    isInMenu = false;
    isSettingTime = false;
    isSettingDate = false;
    isSettingAlarm = false;
    isSettingSys=false;
    isSettingOptions=false;
    isSettingDST=false; // wbp
    blinking = false;
    isSettingHours   = false;
    isSettingMinutes = false;
    isSettingMonth   = false;
    isSettingDay   = false;
    isSettingYear   = false;
    menuItem=0;
    decrement=false;
    for (int i=0;i<MAX_SUBMENUS;i++) { // Initialize SubMenus (wbp)
      subMenu[i]=0;
    }
    okClock=true; 
    isIncrementing = false; 
    wave.stop(); // Any sounds (in case Alarm Tone is playing and sFX are disabled)
    playSFX(3);
    cls();
    isInQMenu=false;
    talkingItem=1; // Reset Talking menu item #
}

