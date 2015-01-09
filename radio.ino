// =======================================================================================
// ---- Receives Temperature from remote RF12B transmitter ----
// =======================================================================================
void receiveTemp() {
  if (!isRadioPresent) return;
  if (isInMenu) return;
  
  if (soundAlarm[0] || soundAlarm[1]) return;
  if (wave.isplaying) return;
  //if (isInQMenu) return;
  if (!radioOn) {
    radio.Wakeup(); // Turn on Radio
    radioOn=true;
    //putstring_nl ("Radio Wake up!");
  }
  if (  ((millis()-last_RF) > 900000 ) || last_RF==0) plot (31,0,RED); //Indicate that temp hasn't been received in over 15 min
  //else if ( (millis()-last_RF) > 5000 ) plot (31,0,BLACK); //Hide indicator
  else  plot (31,0,BLACK); //Hide indicator
  char tempC[3];
  char humidity[3];
  digitalWrite(SS_SD,HIGH); // Disable SD Card / Audio
  if (radio.ReceiveComplete())
    {
       if (radio.CRCPass())
      {
        //Serial.println (now());
        putstring_nl ("Receiving Temp...");
        last_RF=millis();
        plot (31,0,GREEN); //Plot Indicator dot (radio signal was received)
        //Serial.println (radio.GetDataLen());
        /*
        for (byte i = 0; i < *radio.DataLen; i++) {//can also use radio.GetDataLen() if you don't like pointers
          //  Serial.print((char)radio.Data[i]);
          Serial.println (i);  
          if (i==2) ; // Do nothing, it's comma separating temp and humidity
          else {
            if (i<3) tempC[i]=(char)radio.Data[i]; //Fill char array with temp.
            else {// Now we getting humidity 
              humidity[i]=(char)radio.Data[i];
            }
          } // end Else
        } // end For
        */
        if (*radio.DataLen > 2) { // Make sure received data is not empty
          // First 2 characters transmitted are temperature, 3rd is comma, and rest is humidity
          tempC[0]=(char)radio.Data[0]; 
          tempC[1]=(char)radio.Data[1];
          humidity[0]=(char)radio.Data[3];
          humidity[1]=(char)radio.Data[4];
          extTemp = atol(tempC); // Assign external Temp variable Integer number
          extHum = atol(humidity); // Assign external Temp variable Integer number
        }
        //Serial.print ("Temp: "); Serial.println (extTemp);
        //Serial.print ("Humidity: "); Serial.println (extHum);
       
        if (radio.ACKRequested())
        {
          radio.SendACK();
          //Serial.print(" - ACK sent");
        }
      }
      //else
        //Serial.print("BAD-CRC");
      
      //Serial.println();
      
    }
}

// ==================================================
// -- Turns off radio and keeps track of it's status
void turnOffRadio() {
 if (!isRadioPresent) return;
 //radio.Sleep();
 if (radioOn) {
   radio.Sleep();
   radioOn=false;
   //putstring_nl ("Radio Sleep!"); 
  delay (5);
 }
}


