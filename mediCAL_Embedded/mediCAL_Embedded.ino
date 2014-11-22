/* Code based on Adafruit Stepper Libraries and Examples
 * Copyright: EECS Dept, UC Berkeley, 2014
 */

#include <Adafruit_PN532.h>
#include <Wire.h>                      // I2C library
#include <Adafruit_MotorShield.h>
#include <Stepper.h>

// SPI Pins
#define SCK  (2)
#define MOSI (3)
#define SS   (4)
#define MISO (5)

Adafruit_PN532 nfc(SCK, MISO, MOSI, SS);  

/* RFID UIDs - known previously */
uint8_t circle_tag[] = {0x1A, 0xE2, 0x41, 0xD9};
uint8_t bart_tag[] = {0x04, 0x92, 0x6E, 0x7A, 0x7A, 0x31, 0x80};

// Pins for the small stepper motor
int in1Pin = 12;
int in2Pin = 11;
int in3Pin = 10;
int in4Pin = 9;

// hack pins
int RFID1 = 6;
int RFID2 = 7;

Stepper smallMotor(512, in1Pin, in2Pin, in3Pin, in4Pin);  

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
  
// Connect a stepper motor with 200 steps per revolution (1.8 degree)
// to motor port #2 (M3 and M4)
Adafruit_StepperMotor *bigMotor = AFMS.getStepper(200, 2);


void setup() {
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(in3Pin, OUTPUT);
  pinMode(in4Pin, OUTPUT);
  
  Serial.begin(115200);
  smallMotor.setSpeed(30);
  bigMotor->setSpeed(40);  // 40 rpm   

  AFMS.begin();            // create with the default frequency 1.6KHz

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


void loop()
{
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
  /* If a tag is read successfully, send out the UID to the RF board */
  if (success) {
    //nfc.PrintHex(uid, uidLength);  // print it on serial
    for(int i=0; i< uidLength; i++){
      Serial.print('|');
      Serial.print(uid[i], DEC);
    }
    Serial.print(";");
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


  if (Serial.available())
  {
    int steps = Serial.parseInt();
    if(Serial.read() == 'm'){
      Serial.println("SM: Normal steps");
      smallMotor.step(steps);
    }
    else{
      Serial.println("BM: Single coil steps");
      if(steps > 0){
        bigMotor->step(steps, FORWARD, SINGLE); 
      }
      else{
        bigMotor->step(-steps, BACKWARD, SINGLE); 
      }

    }
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
