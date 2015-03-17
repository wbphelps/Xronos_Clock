/////////////////////////////////// Wave Shield Functions
/*
 * print error message and halt
 */
void error_P(const char *str) {
#ifdef PRT_ERROR
  PgmPrint("Error: ");
  SerialPrint_P(str);
#endif
  char string1[20];
  snprintf(string1,sizeof(string1), "Error: $s", str);
  scrolltextsizexcolor(4,string1,RED,5, false);  // scroll without checking buttons (wbp)
  delay(1000);
  sdErrorCheck();
  while(1);
}
/*
 * print error message and halt if SD I/O error, great for debugging!
 */
void sdErrorCheck(void) {
  if (!card.errorCode()) return;
#ifdef PRT_ERROR
  PgmPrint("\r\nSD I/O error: ");
  Serial.print(card.errorCode(), HEX);
  PgmPrint(", ");
  Serial.println(card.errorData(), HEX);
#endif
  scrolltextsizexcolor(4,"SD Card Error",RED,5, false);  // scroll without checking buttons (wbp)
  delay(1000);
  while(1);
}

/*
 * set volume on wave device
 */
static byte waveVol = 99;
void setVol(byte vol) {
  byte vl = MAX_VOLUME-vol;  // volume on wave device is inverted - 0 = loudest
  wave.volume = vl; // Set Playback Sound - 0 is loudest, 7 is lowest
//  delay(100); // try to avoid clicks when volume changed ???
//  Serial.print("setVol("); Serial.print(vol); Serial.print("), "); Serial.print(waveVol); Serial.print("->"); Serial.println(vl);
  waveVol = vl;  // remember last volume (wave.volume is apparently readonly)
}

// =======================================================================================
// ---- Plays a full file from beginning to end with no pause.   ----
// =======================================================================================
// 
void playcomplete(char *name) {
  // call our helper to find and play this name
  turnOffRadio(); // Disable RF12B
  playfile(name);
  while (wave.isplaying) {
  // do nothing while its playing
  }
  // now its done playing
  //radio.Wakeup(); // Re-enable RF12B
}

// =======================================================================================
// ---- Plays a WAVE until new Play requested   ----
// =======================================================================================
// 
void playfile(char *name) {
  turnOffRadio(); // Disable RF12B

  // see if the wave object is currently doing something
  if (wave.isplaying) {// already playing something, so stop it!
    wave.stop(); // stop it
  }
  // look in the root directory and open the file
  if (!f.open(root, name)) {
#ifdef PRT_ERROR
    putstring("Couldn't open file "); 
#endif
    delay (20);
    // Retry
    if (!f.open(root, name)) {
#ifdef PRT_ERROR
      putstring("Couldn't open file "); Serial.print(name);
#endif
      scrolltextsizexcolor(4,"File Error",RED,5, false);  // scroll without checking buttons 
      delay(1000);
      return; 
    }
  }
  
  // OK read the file and turn it into a wave object
  if (!wave.create(f)) {
#ifdef PRT_ERROR
    putstring_nl("Not a valid WAV");
#endif
    return;
  }
  setVol(Settings.soundVol);  // set volume for this file
  // ok time to play! start playback
  wave.play();
 
}

// =======================================================================================
// ---- Plays a WAVE for alarm if nothing currently playing.   ----
// =======================================================================================
void playalarmfile(char *name, byte alrmnum) {
  turnOffRadio(); // Disable RF12B
  // see if the wave object is currently doing something
  if (wave.isplaying) { // already playing something, so just skip
    return;
  }
  // look in the root directory and open the file
  if (!f.open(root, name)) {
#ifdef PRT_ERROR
    putstring("Couldn't open file "); 
#endif
    delay (20);
    // Retry
    if (!f.open(root, name)) {
#ifdef PRT_ERROR
      putstring("Couldn't open file "); Serial.print(name);
#endif
      return; 
    }
  }
  // OK read the file and turn it into a wave object
  if (!wave.create(f)) {
#ifdef PRT_ERROR
    putstring_nl("Not a valid WAV");
#endif
    return;
  }
  // Check if Alarm should be escalated - now done in alarm.ino
//  if (Settings.alarmProgVol[alrmnum]) { // progressive alarm volume?
//    // Escalate alarm volume
//    if (alrmVol[alrmnum]<MAX_VOLUME)  alrmVol[alrmnum]++;
//    setVol(alrmVol[alrmnum]);
//  }
  // ok time to play! start playback
  setVol(alarmVol[alrmnum]);  // set volume for new file
  wave.play();
  //radio.Wakeup(); // Disable RF12B
}

// =======================================================================================
// ---- Initialize Wave Shield and SD card fucntion   ----
// =======================================================================================
void WaveShieldInit () {
 // ===================================================================
 // ------ Wave Shield init
 // if (!card.init(true)) { //play with 4 MHz spi if 8MHz isn't working for you
  if (!card.init()) {         //play with 8 MHz spi (default faster!)  
#ifdef PRT_ERROR
    putstring_nl("Card init. failed!");  // Something went wrong, lets print out why
#endif
    sdErrorCheck();
    while(1);                            // then 'halt' - do nothing!
  }
  // enable optimize read - some cards may timeout. Disable if you're having problems
  card.partialBlockRead(true);
 // Now we will look for a FAT partition!
  uint8_t part;
  for (part = 0; part < 5; part++) {     // we have up to 5 slots to look in
    if (vol.init(card, part)) 
      break;                             // we found one, lets bail
  }
  if (part == 5) {                       // if we ended up not finding one  :(
#ifdef PRT_ERROR
    putstring_nl("No valid FAT partition!");
#endif
    sdErrorCheck();      // Something went wrong, lets print out why
    while(1);                            // then 'halt' - do nothing!
  }
  // Lets tell the user about what we found
#ifdef PRT_DEBUG
  putstring("Using partition ");
  Serial.print(part, DEC);
  putstring(", type is FAT");
  Serial.println(vol.fatType(),DEC);     // FAT16 or FAT32?
#endif
  // Try to open the root directory
  if (!root.openRoot(vol)) {
#ifdef PRT_ERROR
    putstring_nl("Can't open root dir!"); // Something went wrong,
#endif
    while(1);                             // then 'halt' - do nothing!
  }
  // Whew! We got past the tough parts.
#ifdef PRT_DEBUG
  putstring_nl("Ready!");  
#endif
}
