// =======================================================================================
// ---- Check button input and act upon it ----
// Adopted from Florinc by Bratan
// =======================================================================================
static byte button1State = 0;
static byte button2State = 0;
static byte button3State = 0;
//static unsigned long lastButtonTime = 0;// last time button was pushed
static unsigned int buttonHoldTime = 20;  // how long button must be held
// note: only supports one button down at a time, Menu takes priority
// button states:
// 0 - not down
// 1 - down, debouncing
// 2 - released
// 3 - held, waiting for repeat
// 4 - pressed once
// 5 - held, repeat
void checkButtons() {
  if (digitalRead(MENU_BUTTON_PIN) == HIGH) {
    if (button1State == 0) {  // was the button just pressed?
      lastButtonTime = millis();  // start button debounce timer (wbp)
      buttonHoldTime = BUTTON_DEBOUNCE_TIME;  // first hold is just debounce
      button1State = 1;  // button down, waiting for debounce
    }
    else if (button1State > 3)  // did we signal that button was pressed (4) or repeating (5)?
      button1State = 3;  // do that once per interval
    if ((millis() - lastButtonTime) >= buttonHoldTime) {  // has debounce or hold time been reached?
      if (button1State == 1) {  // button just pressed
        button1State = 4;  // signal button pressed once
        buttonHoldTime = BUTTON_HOLD_TIME;  // longer hold before repeat
      }
      else if (button1State == 3) { // button holding for repeat
        button1State = 5;  // signal button repeat
        buttonHoldTime = BUTTON_REPEAT_TIME;  // hold time = repeat time
      }
    }
  }
  else  // button not down
    button1State = 0;  // menu button not pressed

  if (digitalRead(SET_BUTTON_PIN) == HIGH) {
    if (button2State == 0) {  // was the button just pressed?
      lastButtonTime = millis();  // start button debounce timer (wbp)
      buttonHoldTime = BUTTON_DEBOUNCE_TIME;  // first hold is just debounce
      button2State = 1;  // button pressed
    }
    else if (button2State > 3)  // did we signal that button was pressed (4) or repeating (5)?
      button2State = 3;  // do that once per interval
    if ((millis() - lastButtonTime) >= buttonHoldTime) {  // has debounce or hold time been reached?
      if (button2State == 1) {  // button just pressed
        button2State = 4;  // signal button pressed once
        buttonHoldTime = BUTTON_HOLD_TIME;  // longer hold before repeat
      }
      else if (button2State == 3) { // button held, waiting for repeat
        button2State = 5;  // signal button repeating
        buttonHoldTime = BUTTON_REPEAT_TIME;  // hold time = repeat time
      }
    }
  }
  else  // button not down
    button2State = 0;  // menu button not pressed

  if (digitalRead(INC_BUTTON_PIN) == HIGH) {
    if (button3State == 0) {  // was the button just pressed?
      lastButtonTime = millis();  // start button debounce timer (wbp)
      buttonHoldTime = BUTTON_DEBOUNCE_TIME;  // first hold is just debounce
      button3State = 1;  // button pressed
    }
    else if (button3State > 3)  // did we signal that button was pressed (4) or repeating (5)?
      button3State = 3;  // do that once per interval
    if ((millis() - lastButtonTime) >= buttonHoldTime) {  // has debounce or hold time been reached?
      if (button3State == 1) {  // button just pressed
        button3State = 4;  // button down but not held
        buttonHoldTime = BUTTON_HOLD_TIME;  // longer hold before repeat
      }
      else if (button3State == 3) { // button held
        button3State = 5;  // button holding 
        buttonHoldTime = BUTTON_REPEAT_TIME;  // hold time = repeat time
      }
    }
  }
  else {  // button not down
    if (button3State>2)  // was it pressed and debounced?
      button3State = 2;  // button released
    else
      button3State = 0;  // menu button not pressed
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
//      subMenu[0]=0;
//      subMenu[1]=0;
//      subMenu[9]=0; // wbp
}

void buttonProc(){
  // check a few things first
  showAlarm(clockColor); // Display Alarm setting screen
  if (!isInQMenu) mainDate(clockColor); // Show date setting screen, but only if we are not in QMenu
  showSys();
  showOpt();
  showDST(clockColor);

  checkButtons();  // get button status update
  // check MENU button;
  if (button1State>3)  processMenuButton();
  // Check Set button
  if (button2State>3)  processSetButton();
  // If in Menu, check INCR button, check for Menu timeout
  if (isInMenu) {
    if (button3State>3)  processIncButton();
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
    if (button3State==5)  quickDisplay(true);  // button held, do long Quick menu
    else if (lastButtonTime && button3State==2)  quickDisplay(false);  // button released, do short Quick Menu
    if (isInQMenu) {
      if ( (digitalRead(SET_BUTTON_PIN) == HIGH) || (digitalRead(MENU_BUTTON_PIN) == HIGH) ) {
         lastButtonTime = 0; // Exit Quick Menu if any button other than INC was pres
      //processSetButton(); // Only to control alarm
      }
    
      // display the menu option for 10 sec after menu button was pressed;
      if ((lastButtonTime > 0) && (millis() - lastButtonTime < 10000))  // wbp
        return;
  
      // Finished with menus, return to normal operations
      putstring_nl ("Exiting QMenu");
//      cls ();
      exitMenus();
//      okClock=true; // Enable Big Clock display
//      isSettingDate=false;
//      isInQMenu=false;
//      mbutState=1; // Reset Menu Button State
//      isIncrementing = false;
//      blinking = false;
//      isSettingAlarm=false;
//      isSettingDST=false; // wbp
//      menuItem=0; 
//      subMenu[0]=0;
//      subMenu[1]=0;
//      subMenu[9]=0; // wbp
//      mbutState=1;
//      cls();
    }
  }
  
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




