// =======================================================================================
// ---- Check button input and act upon it ----
// Adopted from Florinc by Bratan
// =======================================================================================
static byte button1State = 0;
static byte button2State = 0;
static byte button3State = 0;
static unsigned int buttonHoldTime = 100;  // how long button must be held
void buttonProc(){
  // check a few things first
  showAlarm(clockColor); // Display Alarm setting screen
  if (!isInQMenu) mainDate(clockColor); // Show date setting screen, but only if we are not in QMenu
  showSys();
  showOpt();
  showDST(clockColor);

  // check MENU button;
  if (digitalRead(MENU_BUTTON_PIN) == HIGH) {
    if (button1State == 0) {  // was the button just pressed?
      lastButtonTime = millis();  // start button debounce timer (wbp)
      buttonHoldTime = BUTTON_DEBOUNCE_TIME;  // first hold is just debounce
      button1State = 1;  // button pressed
    }
    if ((millis() - lastButtonTime) < buttonHoldTime)  return;  // debounce
    processMenuButton();
    if (button1State == 1) {  // button just pressed
      button1State = 2;  // button down but not held
      buttonHoldTime = BUTTON_HOLD_TIME;  // longer hold before repeat
    }
    else if (button1State == 2) { // button held
      button1State = 3;  // button holding & repeating
      buttonHoldTime = BUTTON_REPEAT_TIME;  // very short hold now
    }
  }
  else   // button not down
    button1State = 0;  // menu button not pressed

  // Check Set button
  if (digitalRead(SET_BUTTON_PIN) == HIGH) {
    if (button2State == 0) {  // was the button just pressed?
      lastButtonTime = millis();  // start button debounce timer (wbp)
      buttonHoldTime = BUTTON_DEBOUNCE_TIME;  // first hold is just debounce
      button2State = 1;  // button pressed
    }
    if ((millis() - lastButtonTime) < buttonHoldTime)  return;  // debounce, etc
    //if (soundAlarm) interruptAlrm=true; // Stops Alarm sound
    // "Set" button was pressed
    processSetButton();
    if (button2State == 1) {  // button pressed
      button2State = 2;  // button down but not held
      buttonHoldTime = BUTTON_HOLD_TIME;  // longer hold before repeat
    }
    else if (button2State == 2) {  // button held
      button2State = 3;  // button holding & repeating
      buttonHoldTime = BUTTON_REPEAT_TIME;  // very short hold now
    }
  }
  else
    button2State = 0;  // button not pressed

  // If in Menu, INCR buttons
  if (isInMenu) {
    if (digitalRead(INC_BUTTON_PIN) == HIGH) {
      if (button3State == 0) {  // was the button just pressed?
        lastButtonTime = millis();  // start button debounce timer (wbp)
        buttonHoldTime = BUTTON_DEBOUNCE_TIME;  // first hold is just debounce
        button3State = 1;  // button pressed
      }
      if ((millis() - lastButtonTime) < buttonHoldTime)  return;  // debounce, etc
      // "INC" button was pressed. Start changing according values
      processIncButton();
      switch (button3State) {
        case 1:  // button pressed
          button3State = 2;  // button down but not held
          buttonHoldTime = BUTTON_HOLD_TIME;  // longer hold before repeat
          break;
        case 2:  // button held
          button3State = 3;  // button holding & repeating
          buttonHoldTime = BUTTON_REPEAT_TIME;  // very short hold now
          break;
      }
    }
    else
      button3State = 0;  // button not pressed
  
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
//  if (isInMenu) {
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
//    if (isAlarmModified[0]) writeEEPROMAlrm(0);
//    if (isAlarmModified[1]) writeEEPROMAlrm(1);
    wave.stop(); // Any sounds (in case Alarm Tone is playing and sFX are disabled)
    playSFX(3);
    cls();
//  }

  }  // if (isInMenu)
  
}

// =======================================================================================
// ---- Process Menu Button ----
// Adopted from Florinc by Bratan 
// =======================================================================================
void processMenuButton()
{
  if (isInQMenu) return; // We are in quick menu, so don't show settings menu
    
 // ====  Alarm control   ====
  if ( soundAlarm[0] || soundAlarm[1] || (snoozeTime[0]<10) || (snoozeTime[1]<10) ) {  // sounding alarm or snoozing?
    if ((millis() - lastButtonTime) < RESET_BUTTON_TIME)  return;  // hold button down to reset alarm
    if (resetAlrm(0) || resetAlrm(1) ) return;
  }
 
  //putstring_nl ("Is In Menu Button Proc");
  lastButtonTime = millis();
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
// ---------------------------------------------------------------------------------------



// =======================================================================================
// ---- Process Set Button ----
// Adopted from Florinc by Bratan (added date setting)
// =======================================================================================
void processSetButton()
{
  
  // If Alarms sound it will stop them and snooze
  snoozeProc(0);
  snoozeProc(1);
 
  if (!isInMenu) return; // failsafe
  lastButtonTime = millis();
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
void processIncButton()
{  
  lastButtonTime = millis();
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




