// This code is for the nRF51822 that is used to provide a BLE interface to PILLar
// There is 1 service and 1 characteristic
// The service provides a way to write a step value from the phone to a stepper motor
// controlled by an Arduino
#include "mbed.h"
#include "BLEDevice.h"

BLEDevice  ble;                 // Declare the ble device
Serial arduino(USBTX, USBRX);   // serial interface to PC or Arduino controlling the stepper

// Arbit 128-bit UIDs generated from 
// http://www.guidgenerator.com/online-guid-generator.aspx
// We need one for each service and characteristic
const uint8_t ARDUINO_WRITE_CHARACTERISTIC_UUID[LENGTH_OF_LONG_UUID] = {
    0xfb, 0x71, 0xbc, 0xc0, 0x5a, 0x0c, 0x11, 0xe4,
    0x91, 0xae, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b
};

const uint8_t TEST_SERVICE_UUID[LENGTH_OF_LONG_UUID] = {
    0xb0, 0xbb, 0x58, 0x20, 0x5a, 0x0d, 0x11, 0xe4,
    0x93, 0xee, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b};

const static char DEVICE_NAME[] = "mediCAL BLE";        // For Advertisement    
static volatile uint16_t writeToArduino_handler;        // handler to echo data

// What happens on disconnect event
void disconnectionCallback(Gap::Handle_t handle, Gap::DisconnectionReason_t reason){
    ble.startAdvertising(); // restart advertising
}

// This function is called whenever data is written to the BLE device through the phone
void onDatawritten(const GattCharacteristicWriteCBParams *eventDataP) {
    arduino.printf("Enter onDataWritten\n\r");
    // eventDataP->charHandle is just uint16_t
    // it's used to dispatch the callbacks
    if(eventDataP->charHandle==writeToArduino_handler){         // The data is for the motor
        arduino.printf("Got some Arduino-specific info!\n\r");
        uint16_t bytesRead = eventDataP->len;
        int turnSteps = *((int16_t *)eventDataP->data);
        arduino.printf("%i\n\r", turnSteps); 
    }
}

int main(void)
{
    arduino.printf("Entered main\n\r");

    // You can write from the phone to control what is written to Arduino
    GattCharacteristic writeToArduino_characteristics(
        ARDUINO_WRITE_CHARACTERISTIC_UUID, NULL, sizeof(int16_t), sizeof(int16_t),
        GattCharacteristic::BLE_GATT_FORMAT_SINT16 |                                        //16bit signed INT
        GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |                                 // has read
        GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE);                                // and write properties
    writeToArduino_handler = writeToArduino_characteristics.getValueAttribute().getHandle();// save the handler

    GattCharacteristic *charTable[] = {&writeToArduino_characteristics};                    // Characteristic table
    GattService testService(TEST_SERVICE_UUID, charTable,                                   // Add the char(s) to this service
                            sizeof(charTable) / sizeof(GattCharacteristic *));

    // BLE setup, mainly we add service and callbacks
    ble.init();
    ble.addService(testService);
    ble.onDataWritten(&onDatawritten);
    ble.onDisconnection(disconnectionCallback);
    
    // setup advertising
    ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED |
                                     GapAdvertisingData::LE_GENERAL_DISCOVERABLE);                      
    ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME,
                                     (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.setAdvertisingInterval(1600); /* 1000ms; in multiples of 0.625ms. */
    ble.startAdvertising();

    while (true) {
            arduino.printf("Waiting for ble Event\n\r");
            ble.waitForEvent();
    }
}