mediCal's Pillar - Your Personal Pill Assistant
=======

This repository contains the mBED proejct for providing hardware support to PILLAR (automatic pill dispenser) by team mediCAL.

This project is being developed as a part of the Interactive Device Design (Fall '14) class at UC Berkeley

- BLE code is in the BLE_Medical folder: The BLE chip used is the nRF51822. The project is based on Ben Zhang's BLE_API_DEMO mBED project (https://developer.mbed.org/users/nebgnahz/code/BLE_API_DEMO/)

- Stepper and RFID code is in the readMifare_Stepper folder: An arduino is used alongside the PN532 NFC reader and a L293D H-bridge motor driver to achieve functionality. This project is based on Adafruit's MiFare Arduino library/example and stepper code from here (https://learn.adafruit.com/adafruit-arduino-lesson-16-stepper-motors?view=all)

See: https://github.com/defond0/mediCal for the Android app used to control this hardware
