mediCal's Pillar - Your Personal Pill Assistant
=======

This repository contains the mBED proejct for providing hardware support to PILLAR (automatic pill dispenser) by team mediCAL.

This project is being developed as a part of the Interactive Device Design (Fall '14) class at UC Berkeley

- The complete RFID/Stepper Arduino code is in the mediCAL_Embedded folder. This code achieves the following:
  * Read RFID tag
  * Send RFID tag over serial to BLE device
  * Listen over serial to determine what pill to dispense
  * Based on pill/funnel number, move 2 stepper motors to appropriate positions
 Hardware: Arduino UNO, PN532 NFC reader, L293D H-bridge motor driver, 74HC4050 level shifter and the Adafruit motor shield v2. This project is uses Adafruit's MiFare Arduino library/example and stepper code from here (https://learn.adafruit.com/adafruit-arduino-lesson-16-stepper-motors?view=all)

- BLE code is in the BLE_Medical folder: The BLE chip used is the nRF51822. The project is based on Ben Zhang's BLE_API_DEMO mBED project (https://developer.mbed.org/users/nebgnahz/code/BLE_API_DEMO/). This code does the following:
  * Echo RFID tag read from Arduino over BLE
  * Send out the funnel number to dispense over serial to the Arduino

- Old Stepper and RFID code is in the readMifare_Stepper folder

See: https://github.com/defond0/mediCal for the Android app used to control this hardware

Fun links:
* Project StoryBoard: http://husk.eecs.berkeley.edu/courses/cs294-84-fall14/index.php/ObservationStoryboard-Group:MediCAL
* Prototype 1: http://husk.eecs.berkeley.edu/courses/cs294-84-fall14/index.php/P3-mediCAL#Team_mediCAL
* Prototype 1.1: http://husk.eecs.berkeley.edu/courses/cs294-84-fall14/index.php/Homework5-mediCAL
* Prototype 2.0: http://husk.eecs.berkeley.edu/courses/cs294-84-fall14/index.php/P4-MediCAL
* Prototype 2.5: https://www.youtube.com/watch?v=617ZXK32iUA and https://www.youtube.com/watch?v=oQ7gYv67zXA 
