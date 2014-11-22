/* 
This is a test sketch for the Adafruit assembled Motor Shield for Arduino v2
It won't work with v1.x motor shields! Only for the v2's with built in PWM
control

For use with the Adafruit Motor Shield v2 
---->	http://www.adafruit.com/products/1438
*/


#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_PWMServoDriver.h"
#include <Stepper.h>

int in1Pin = 12;
int in2Pin = 11;
int in3Pin = 10;
int in4Pin = 9;

Stepper smallMotor(512, in1Pin, in2Pin, in3Pin, in4Pin);  

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61); 

// Connect a stepper motor with 200 steps per revolution (1.8 degree)
// to motor port #2 (M3 and M4)
Adafruit_StepperMotor *bigMotor = AFMS.getStepper(200, 2);


void setup() {
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(in3Pin, OUTPUT);
  pinMode(in4Pin, OUTPUT);
  
  Serial.begin(9600);
  smallMotor.setSpeed(30);
  bigMotor->setSpeed(40);  // 40 rpm   

  AFMS.begin();  // create with the default frequency 1.6KHz
  //AFMS.begin(1000);  // OR with a different frequency, say 1KHz  
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
//
//  Serial.println("Double coil steps");
//  bigMotor->step(100, FORWARD, DOUBLE); 
//  bigMotor->step(100, BACKWARD, DOUBLE);
//  
//  Serial.println("Interleave coil steps");
//  bigMotor->step(100, FORWARD, INTERLEAVE); 
//  bigMotor->step(100, BACKWARD, INTERLEAVE); 
//  
//  Serial.println("Microstep steps");
//  bigMotor->step(50, FORWARD, MICROSTEP); 
//  bigMotor->step(50, BACKWARD, MICROSTEP);

    }
  }
}
