// This code is for the nRF51822 that is used to provide a BLE interface to PILLar
// There is 1 service and 2 characteristics
// Characteristic 1 provides a way to write funnel number to dispense from
// Characteristic 2 sends back RFID tag that is read

#include "mbed.h"
#include "BLEDevice.h"
#define RFID_BUFF_SIZE  8       // Enough to hold 7-byte UID + delimiter
#define ONE_REVOLUTION_SMALL 513
#define ONE_REVOLUTION_BIG 200

DigitalOut led1(LED1);          // LED toggled on connect and write events
DigitalOut led2(LED2);          // Toggled on button press
BLEDevice  ble;                 // Declare the ble device
Serial arduino(USBTX, USBRX);   // serial interface to PC or Arduino controlling the stepper

// Arbit 128-bit UIDs generated from
// http://www.guidgenerator.com/online-guid-generator.aspx
// We need one for each service and characteristic
const uint8_t ARDUINO_WRITE_CHARACTERISTIC_UUID[LENGTH_OF_LONG_UUID] = {
    0xfb, 0x71, 0xbc, 0xc0, 0x5a, 0x0c, 0x11, 0xe4,
    0x91, 0xae, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b
};

const uint8_t RFID_UUID[LENGTH_OF_LONG_UUID] = {
    0x7a, 0x77, 0xbe, 0x20, 0x5a, 0x0d, 0x11, 0xe4,
    0xa9, 0x5e, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b
};

const uint8_t TEST_SERVICE_UUID[LENGTH_OF_LONG_UUID] = {
    0xb0, 0xbb, 0x58, 0x20, 0x5a, 0x0d, 0x11, 0xe4,
    0x93, 0xee, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b};

const static char DEVICE_NAME[] = "mediCAL BLE";        // For Advertisement
static volatile uint16_t chooseFunnel_handler;        // handler for input data (funnel number)
static volatile bool is_button_pressed = false;
static volatile bool uid_read = false;

// What happens on connect event - DEBUG purposes ONLY!!
void onConnection(Gap::Handle_t handle, const Gap::ConnectionParams_t * param_ptr){
    arduino.printf("Connection Event!\n\r");
    ble.stopAdvertising(); // stop advertising
    led1 = !led1.read();
}

// What happens on disconnect event
void disconnectionCallback(Gap::Handle_t handle, Gap::DisconnectionReason_t reason){
    arduino.printf("Disconnection Event!\n\r");
    ble.startAdvertising(); // restart advertising
}

// This function is called whenever data is written to the BLE device through the phone
void onDatawritten(const GattCharacteristicWriteCBParams *eventDataP) {
    // eventDataP->charHandle is just uint16_t
    // it's used to dispatch the callbacks
    if(eventDataP->charHandle==chooseFunnel_handler){         // The data is for the Arduino controlling the motor
        uint16_t bytesRead = eventDataP->len;                   // may not be required
        // What the BLE master will tell us is which funnels to dispense from
        int funnel = *((int16_t *)eventDataP->data);
        switch(funnel){
                //            190m large to circle
                //182m circle to small
                //140m small to large
                
            case 0:
                arduino.printf("%dM", ONE_REVOLUTION_BIG/2);             // move funnel to hole
                arduino.printf("%dm", ONE_REVOLUTION_SMALL/2);             // move hole to right position
                arduino.printf("%dM", ONE_REVOLUTION_BIG/4);             // move funnel to hole
                arduino.printf("%dm", ONE_REVOLUTION_SMALL/2);             // move hole to dispense
                arduino.printf("%dm", -ONE_REVOLUTION_SMALL/2);            // move hole back
                arduino.printf("%dm", -ONE_REVOLUTION_SMALL/2);            // move hole to original position
                arduino.printf("%dM", ONE_REVOLUTION_BIG/4);             // move funnel to back
                break;
            case 1:
                arduino.printf("%dm", 137);
                arduino.printf("%dM", 75);
                arduino.printf("%dM", -75);
                arduino.printf("%dm", 512);
                arduino.printf("%dm", -512);
                arduino.printf("%dm", -137);
                break;
            case 2:
                arduino.printf("%dm", -134);
                arduino.printf("%dM", 125);
                arduino.printf("%dM", -125);
                arduino.printf("%dm", 512);
                arduino.printf("%dm", -512);
                arduino.printf("%dm", 134);
                break;
                
            case 3:
                arduino.printf("%dm", 384);
                arduino.printf("%dM", 175);
                arduino.printf("%dM", -175);
                arduino.printf("%dm", 128);
                arduino.printf("%dm", -128);
                arduino.printf("%dm", -384);
                break;
                
            default:
                break;
        }
        
        led1 = !led1.read();                                    // debug purposes
    }
}
char read_ch;                                   // temp var to store data from arduino
uint8_t buff[RFID_BUFF_SIZE] = {0};             // max packet size for GATT is 20 bytes
uint8_t ct = 0;

void callback() {
    // Note: you need to actually read from the serial to clear the RX interrupt
    read_ch = arduino.getc();
    buff[ct++] = int(read_ch);
    
    if((ct > RFID_BUFF_SIZE-1)){                // Arduino sends 8-bytes [UID][;][0 padding]
        uid_read = true;
        led2 = !led2.read();                        // debug purposes
    }
}

// Button1 pressed routine - toggle is_button pressed
void button1Pressed() {
    is_button_pressed = !is_button_pressed;
    led2 = !led2.read();                        // debug
}

// Button2 pressed routine
void button2Pressed() {
    led2 = !led2.read();                        // debug
}

int main(void)
{
    arduino.baud(115200);
    arduino.printf("Entered main\n\r");
    
    // RFID Interrupt initialization
    // button initialization
    InterruptIn button1(BUTTON1);
    button1.mode(PullUp);
    button1.rise(&button1Pressed);
    
    InterruptIn button2(BUTTON2);
    button2.mode(PullUp);
    button2.rise(&button2Pressed);
    
    arduino.attach(&callback);                  // callback is triggered when there is incoming serial data
    
    // You can write from the phone to control what is written to Arduino
    GattCharacteristic chooseFunnel_characteristics(
                                                    ARDUINO_WRITE_CHARACTERISTIC_UUID, NULL, sizeof(int16_t), sizeof(int16_t),
                                                    GattCharacteristic::BLE_GATT_FORMAT_SINT16 |                                        // 16-bit signed int
                                                    GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |                                 // has read
                                                    GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE);                                // and write properties
    chooseFunnel_handler = chooseFunnel_characteristics.getValueAttribute().getHandle();// save the handler
    
    GattCharacteristic RFID_characteristics(
                                            RFID_UUID, NULL, RFID_BUFF_SIZE*sizeof(uint8_t), RFID_BUFF_SIZE*sizeof(uint8_t),
                                            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
                                            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
    
    
    GattCharacteristic *charTable[] = {&chooseFunnel_characteristics,
        &RFID_characteristics};                    // Characteristic table
    GattService testService(TEST_SERVICE_UUID, charTable,                                   // Add the char(s) to this service
                            sizeof(charTable) / sizeof(GattCharacteristic *));
    
    // BLE setup, mainly we add service and callbacks
    ble.init();
    ble.addService(testService);
    ble.onDataWritten(onDatawritten);
    ble.onConnection(onConnection);
    ble.onDisconnection(disconnectionCallback);
    
    // setup advertising
    ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED |
                                     GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME,
                                     (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.setAdvertisingInterval(160); /* 100ms; in multiples of 0.625ms. */
    ble.startAdvertising();
    
    while (true) {
        if (uid_read) {
            ble.updateCharacteristicValue(RFID_characteristics.getValueAttribute().getHandle(),
                                          buff, RFID_BUFF_SIZE*sizeof(uint8_t));
            uid_read = false;
            ct = 0;
        } 
        else {
            ble.waitForEvent();        
        }
    }
}