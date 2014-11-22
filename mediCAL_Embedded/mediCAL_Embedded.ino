/* Code based on Adafruit Stepper Libraries and Examples
 * Copyright: EECS Dept, UC Berkeley, 2014
 */

#include <Wire.h>                      // I2C library
#include <Adafruit_MotorShield.h>
#include <Stepper.h>

// Pins for the small stepper motor
int in1Pin = 12;
int in2Pin = 11;
int in3Pin = 10;
int in4Pin = 9;

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
}


void loop()
{
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
