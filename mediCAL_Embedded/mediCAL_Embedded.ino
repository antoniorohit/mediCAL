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

#define ONE_REVOLUTION_BIG  200                      // steps per revolution
#define ONE_REVOLUTION_SMALL  513                      // steps per revolution

#define SMALL_MOTOR_SPEED    30
#define BIG_MOTOR_SPEED      30

Adafruit_PN532 nfc(SCK, MISO, MOSI, SS);  

/* RFID UIDs - known previously */
uint8_t circle_tag[] = {0x1A, 0xE2, 0x41, 0xD9};
uint8_t bart_tag[] = {0x04, 0x92, 0x6E, 0x7A, 0x7A, 0x31, 0x80};
uint8_t orca_tag[] = {0x04, 0x41, 0x18, 0x7A, 0x3D, 0x22, 0x80};
uint8_t simple_rect_tag[] = {0xAA, 0x79, 0x9B, 0x23};

// Pins for the small stepper motor
int in1Pin = 12;
int in2Pin = 11;
int in3Pin = 10;
int in4Pin = 9;

// 3 buttons - limit switches for each motor and cup
int smallMotorSwitch = 6;
int bigMotorSwitch = 7;
int cupSwitch = 8;

Stepper smallMotor(ONE_REVOLUTION_SMALL, in1Pin, in2Pin, in3Pin, in4Pin);  

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
  
// Connect a stepper motor with 360 steps per revolution (1 degree)
// to motor port #2 (M3 and M4)
Adafruit_StepperMotor *bigMotor = AFMS.getStepper(ONE_REVOLUTION_BIG, 2);

// Absolute Motor Positions
int currPos_smallMotor = 361;      // initialize with an illegal motor position
int currPos_bigMotor = 361;        // initialize with an illegal motor position

void setup() {
  // Small motor pins
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(in3Pin, OUTPUT);
  pinMode(in4Pin, OUTPUT);
  
  // switches
  pinMode(bigMotorSwitch, INPUT_PULLUP);     // sets the digital pin 6 as input
  pinMode(smallMotorSwitch, INPUT_PULLUP);   // sets the digital pin 7 as input
  pinMode(cupSwitch, INPUT_PULLUP);          // enable pullup on pin 8

  Serial.begin(115200);
  smallMotor.setSpeed(SMALL_MOTOR_SPEED);
  bigMotor->setSpeed(BIG_MOTOR_SPEED);  // rpm   

  AFMS.begin();
  // create with the default frequency 1.6KHz

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
  Serial.print("Success");
  callibrateSmallMotor();
  callibrateBigMotor();
}


void loop()
{
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  char readChar;
  
  
  if(digitalRead(cupSwitch) == 0){
    // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
    // 'uid' will be populated with the UID, and uidLength will indicate
    // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
    
    /* If a tag is read successfully, send out the UID to the RF board */
    if (success) {
      for(int i=0; i< uidLength; i++){
        Serial.write(char(uid[i]));      
      }
      if(uidLength < 7){
        Serial.write(0);
        Serial.write(0);
        Serial.write(0);
      }
      Serial.write(';');
    }
  }

  if (Serial.available())
  {
    int steps = Serial.parseInt();
    readChar = Serial.read();
    if(readChar == 'm'){
      smallMotor.step(steps);
      Serial.print("SM Curr Pos: ");
      if(currPos_smallMotor + steps < 0){
        currPos_smallMotor = (currPos_smallMotor + steps + ONE_REVOLUTION_SMALL);
      }
      else{
        currPos_smallMotor = (currPos_smallMotor + steps)%ONE_REVOLUTION_SMALL;
      }
      Serial.print(currPos_smallMotor, DEC);
      Serial.println();
      if((currPos_smallMotor == 0) || (currPos_smallMotor == 512) || (currPos_smallMotor == 1)){
          callibrateSmallMotor();
      }
    }
    else if(readChar == 'M'){
//      jerkMotor(bigMotor);
      if(steps > 0){
        bigMotor->step(steps, FORWARD, DOUBLE); 
        currPos_bigMotor = (currPos_bigMotor+steps)%ONE_REVOLUTION_BIG;
      }
      else{
        bigMotor->step(-steps, BACKWARD, DOUBLE); 
        if(currPos_bigMotor + steps > 0){
          currPos_bigMotor = (currPos_bigMotor + steps);
        }
         else{
           currPos_bigMotor = (currPos_bigMotor + steps + ONE_REVOLUTION_BIG)%ONE_REVOLUTION_BIG;
         }        
    }

      if(currPos_bigMotor == 0){
        callibrateBigMotor();  
      }
      
      Serial.print("BM Curr Pos: ");
      Serial.print(currPos_bigMotor, DEC);
      Serial.println();
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

void jerkMotor(Adafruit_StepperMotor *motor)
{
  int steps = 4;
//  motor->setSpeed(50);  // 100 rpm   
//  motor->step(steps, FORWARD, DOUBLE); 
//  motor->step(steps, BACKWARD, DOUBLE);
//  motor->step(steps, FORWARD, DOUBLE); 
//  motor->step(steps, BACKWARD, DOUBLE);
//  motor->step(steps, FORWARD, DOUBLE); 
//  motor->step(steps, BACKWARD, DOUBLE);
//  motor->step(steps, FORWARD, DOUBLE); 
//  motor->step(steps, BACKWARD, DOUBLE);
//  motor->step(steps, FORWARD, DOUBLE); 
//  motor->step(steps, BACKWARD, DOUBLE);
//  motor->step(steps, FORWARD, DOUBLE); 
//  motor->step(steps, BACKWARD, DOUBLE);
  motor->setSpeed(SMALL_MOTOR_SPEED);  // 100 rpm   
}

// Callibrate Small Motor by turning until the limit switch is hit
void callibrateSmallMotor()
{
  int i = 0;

  smallMotor.setSpeed(SMALL_MOTOR_SPEED);
  while(digitalRead(smallMotorSwitch))
  {
    smallMotor.step(-2);
    i--;
    if(i < -ONE_REVOLUTION_SMALL/12){
        smallMotor.step(10);  // Compensation turn
        break;
    }  // more than 30 deg
  }
  smallMotor.step(10);  // Compensation turn
  while(digitalRead(smallMotorSwitch))
  {
    smallMotor.step(2);
    i++;
    if(i > ONE_REVOLUTION_SMALL/2){
      break;  // more than 1 revolution
    }
  }
  
  smallMotor.step(-10);  // Compensation turn

  Serial.write("\n\rSmall motor callibrated");
  smallMotor.setSpeed(SMALL_MOTOR_SPEED);
  currPos_smallMotor = 0;
}

void callibrateBigMotor()
{
  int i = 0;
  while(digitalRead(bigMotorSwitch))
  {
    bigMotor->step(2, FORWARD, DOUBLE);
    i++;
    if(i > ONE_REVOLUTION_BIG)  break;  // more than 2 revolutions
  }
  
  bigMotor->step(ONE_REVOLUTION_BIG/8, BACKWARD, DOUBLE);    // Rest position of the funnel is 45 degrees off the limit switch

  Serial.write("\n\rBig motor callibrated");
  currPos_bigMotor = 0;
}


