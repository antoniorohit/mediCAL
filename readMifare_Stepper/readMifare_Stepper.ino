/* Code for HW5 of IDD
 * 2 basic functions are implemented - RFID reader and stepper driving
 * Due to time constraint, the RFID reader is implemented in a hacky way:
 * Only 2 RFID tags are 'stored' in the local Arduino database = circle_tag 
 * and bart_tag. If circle_tag is read, a digital out (RFID1) is toggled - this
 * digital out is connected to the nRF51822 to signal a RFID read event
 * Note that we use Digital out instead of serial to communicate with the 
 * nRF51822 for legacy purposes - the buttons on the RF board were used 
 * previously to emulate RFID tag read events, so it was easiest to just
 * connect to those pins and see if everything worked. In the near future, 
 * those pins will be removed.
 * Similarly, if the bart_tag is read, RFID2 is toggled.
 * The Arduino also relies on the RF baord to tell it how much to spin the
 * motor
 */ 

#include <Adafruit_PN532.h>
#include <Stepper.h>

#define SCK  (2)
#define MOSI (3)
#define SS   (4)
#define MISO (5)

/* RFID UIDs - known previously */
uint8_t circle_tag[] = {0x1A, 0xE2, 0x41, 0xD9};
uint8_t bart_tag[] = {0x04, 0x92, 0x6E, 0x7A, 0x7A, 0x31, 0x80};

Adafruit_PN532 nfc(SCK, MISO, MOSI, SS);

// RFID pins (SPI)
int in1Pin = 12;
int in2Pin = 11;
int in3Pin = 10;
int in4Pin = 9;

// hack pins
int RFID1 = 6;
int RFID2 = 7;

// motor driver pins (these go to the L293D Hbridge driver)
Stepper motor(512, in1Pin, in2Pin, in3Pin, in4Pin);  

void setup(void) {
  /* Stepper pins */
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(in3Pin, OUTPUT);
  pinMode(in4Pin, OUTPUT);
  
  /* Hack button emulator pins - when either of these 
     are activated, the RF board writes a particular UID to the
     BLE notify characteristic.
   */
  pinMode(RFID1, OUTPUT);
  pinMode(RFID2, OUTPUT);

  motor.setSpeed(20);
  
  Serial.begin(9600);

  nfc.begin();

  /* @TODO Verify if this code snippet works properly */
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1){
      nfc.begin();
      versiondata = nfc.getFirmwareVersion();
      if(versiondata)  break;
      delay(1);
    }
  }
  
  // configure board to read RFID tags
  nfc.SAMConfig();
}


void loop(void) {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
  /* If a tag is read successfully, send out the UID to the RF board */
  if (success) {
    nfc.PrintHex(uid, uidLength);  // print it on serial
    // but for now, also send it using the hack method
    if(isEqual(uid, circle_tag, uidLength)){
        digitalWrite(RFID1, 1);
        digitalWrite(RFID2, 0);
    }
    else if(isEqual(uid, bart_tag, uidLength)){
        digitalWrite(RFID1, 0);
        digitalWrite(RFID2, 1);
    }
    else{
        digitalWrite(RFID1, 0);
        digitalWrite(RFID2, 0);
    }
  }
  
  // listen for motor turn packets
  if (Serial.available())
  {
    int steps = Serial.parseInt();
    motor.step(steps);
  }
  
}

// helper function to check whether two UIDs are equal
boolean isEqual(uint8_t *ID1, uint8_t *ID2, uint8_t length)
{
  boolean res = true;
  for(int i=0; i<length; i++){
    if(ID1[i] != ID2[i]){
      res = false;
      break;
    }
  }
  return res;
}
