// System Definitions
#define IR_PIN 11 // IR Receiver Pin (D11)


// Custom IR Codes defintions
// ---- Avon Mini remote ----
/*
#define IR_ON  0x17E18E7 // On/Off
#define IR_MINUS  0x17EA857 
#define IR_PLUS  0x17E28D7 
#define IR_DOWN  0x17E00FF 
#define IR_UP  0x17E40BF 
#define IR_ENTER  0x17EAE51 
#define IR_TALK  0x17E9867 
*/
// ---- Keychain Mini remote -----
#define IR_ON  0x20DF10EF // On/Off
#define IR_UP  0x20DF00FF // Channel Up button
#define IR_DOWN  0x20DF807F  // Channel Down button
#define IR_MINUS  0x20DFC03F // Vol - 
#define IR_PLUS  0x20DF40BF  // Vol +
#define IR_ENTER  0x20DFD02F // AV/TV button
#define IR_TALK  0x20DF906F // Mute button

// IR Receiver init (comment out following 2 lines if IR receiver not present!
IRrecv irrecv(IR_PIN);
decode_results results;


