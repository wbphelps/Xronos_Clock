// =======================================================================================
// ---- Check button input and act upon it ----
// Adopted from Florinc by Bratan
// =======================================================================================
void buttonProc(){
  // check MENU button;
  showAlarm(clockColor); // Display Alarm setting screen
  if (!isInQMenu) mainDate(clockColor); // Show date setting screen, but only if we are not in QMenu
  showSys();
  showOpt();
  showDST(clockColor);
  if (digitalRead(MENU_BUTTON_PIN) == HIGH)
  {
    processMenuButton();
  }
   // check SET button;
  if (isInMenu)
  {
    if (digitalRead(SET_BUTTON_PIN) == HIGH)
    {
      //if (soundAlarm) interruptAlrm=true; // Stops Alarm sound
      // "Set" button was pressed. Go into this level of menu and stay until exit form it or timeout
      processSetButton();
     }
     if (digitalRead(INC_BUTTON_PIN) == HIGH)
    {
      // "INC" button was pressed. Start changing according values
      processIncButton();
     }
  }
  // display the menu option for 6 seconds after menu button was pressed;
  if ((lastButtonTime > 0) && (millis() - lastButtonTime < 6000)) {
    if ( millis() - lastButtonTime > 1000 )  { // Start blinking if buttons not touched for a second
       isIncrementing = false;
      if (!isInQMenu) startBlinking();
    }
    return;
  }

  // return the main mode if no button was pressed for 5 seconds;
   if (isInMenu)
  {
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
//   for (int i=0;i<MAX_SUBMENUS+1;i++) { // Initialize SubMenus
   for (int i=0;i<MAX_SUBMENUS;i++) { // Initialize SubMenus (wbp)
      subMenu[i]=0;
   }
   
    okClock=true; 
    isIncrementing = false; 
    if (isAlarmModified[0]) writeEEPROMAlrm(0);
    if (isAlarmModified[1]) writeEEPROMAlrm(1);
    wave.stop(); // Any sounds (in case Alarm Tone is playing and sFX are disabled)
    playSFX(3);
    cls();
  }
  
}

// =======================================================================================
// ---- Process Menu Button ----
// Adopted from Florinc by Bratan 
// =======================================================================================
void processMenuButton()
{
  if (isInQMenu) return; // We are in quick menu, so don't show settings menu
  // debouncing;
  if ((millis() - lastButtonTime) < BOUNCE_TIME_BUTTON)
    return;
    
 // ====  Alarm control   ====
 if (resetAlrm(0) || resetAlrm(1) ) return;
 
  //putstring_nl ("Is In Menu Button Proc");
  lastButtonTime = millis();
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
  
  // debouncing;
  if ((millis() - lastButtonTime) < BOUNCE_TIME_BUTTON)
    return;
 
  // If Alarms sound it will stop them and snooze
  snoozeProc(0);
  snoozeProc(1);
 
  if (!isInMenu) return; // failsafe
  lastButtonTime = millis();
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
  // debouncing;
  if ((millis() - lastButtonTime) < BOUNCE_TIME_BUTTON)
    return;
  isIncrementing = true;
  stopBlinking();
  lastButtonTime = millis();
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




