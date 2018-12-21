/*****************************************************************************
data_saving.ino
Written By:  Sara Damiano (sdamiano@stroudcenter.org)
Development Environment: PlatformIO
Hardware Platform: EnviroDIY Mayfly Arduino Datalogger
Software License: BSD-3.
  Copyright (c) 2017, Stroud Water Research Center (SWRC)
  and the EnviroDIY Development Team

This example sketch is written for ModularSensors library version 0.19.2

This sketch is an example of logging data to an SD card and sending only a
portion of that data to the EnviroDIY data portal.

DISCLAIMER:
THIS CODE IS PROVIDED "AS IS" - NO WARRANTY IS GIVEN.
*****************************************************************************/

// ==========================================================================
//    Include the base required libraries
// ==========================================================================
#include <Arduino.h>  // The base Arduino library
#include <EnableInterrupt.h>  // for external and pin change interrupts


// ==========================================================================
//    Data Logger Settings
// ==========================================================================
// The library version this example was written for
const char *libraryVersion = "0.19.2";
// The name of this file
const char *sketchName = "data_saving.ino";
// Logger ID, also becomes the prefix for the name of the data file on SD card
const char *LoggerID = "XXXXX";
// How frequently (in minutes) to log data
const uint8_t loggingInterval = 5;
// Your logger's timezone.
const int8_t timeZone = -5;  // Eastern Standard Time
// NOTE:  Daylight savings time will not be applied!  Please use standard time!


// ==========================================================================
//    Primary Arduino-Based Board and Processor
// ==========================================================================
#include <sensors/ProcessorStats.h>

const long serialBaud = 115200;   // Baud rate for the primary serial port for debugging
const int8_t greenLED = 8;        // MCU pin for the green LED (-1 if not applicable)
const int8_t redLED = 9;          // MCU pin for the red LED (-1 if not applicable)
const int8_t buttonPin = 21;      // MCU pin for a button to use to enter debugging mode  (-1 if not applicable)
const int8_t wakePin = A7;        // MCU interrupt/alarm pin to wake from sleep
// Set the wake pin to -1 if you do not want the main processor to sleep.
// In a SAMD system where you are using the built-in rtc, set wakePin to 1
const int8_t sdCardPin = 12;      // MCU SD card chip select/slave select pin (must be given!)
const int8_t sensorPowerPin = 22; // MCU pin controlling main sensor power (-1 if not applicable)

// Create and return the processor "sensor"
const char *MFVersion = "v0.5b";
ProcessorStats mayfly(MFVersion);
// Create the battery voltage and free RAM variable objects for the processor and return variable-type pointers to them
Variable *mayflyBatt = new ProcessorStats_Batt(&mayfly, "12345678-abcd-1234-efgh-1234567890ab");
Variable *mayflyRAM = new ProcessorStats_FreeRam(&mayfly, "12345678-abcd-1234-efgh-1234567890ab");
Variable *mayflySampNo = new ProcessorStats_SampleNumber(&mayfly, "12345678-abcd-1234-efgh-1234567890ab");


// ==========================================================================
//    Modem/Internet connection options
// ==========================================================================

// Select your modem chip
#define TINY_GSM_MODEM_SIM800  // Select for a SIM800, SIM900, or variant thereof

// Include TinyGSM for the modem
// This include must be included below the define of the modem name!
#include <TinyGsmClient.h>

 // Set the serial port for the modem - software serial can also be used.
HardwareSerial &ModemSerial = Serial1;

// Create a new TinyGSM modem to run on that serial port and return a pointer to it
TinyGsm *tinyModem = new TinyGsm(ModemSerial);

// Use this to create a modem if you want to spy on modem communication through
// a secondary Arduino stream.  Make sure you install the StreamDebugger library!
// https://github.com/vshymanskyy/StreamDebugger
// #include <StreamDebugger.h>
// StreamDebugger modemDebugger(Serial1, Serial);
// TinyGsm *tinyModem = new TinyGsm(modemDebugger);

// Create a new TCP client on that modem and return a pointer to it
TinyGsmClient *tinyClient = new TinyGsmClient(*tinyModem);

// Describe the physical pin connection of your modem to your board
const long ModemBaud = 9600;         // Communication speed of the modem
const int8_t modemVccPin = -2;       // MCU pin controlling modem power (-1 if not applicable)
const int8_t modemSleepRqPin = 23;   // MCU pin used for modem sleep/wake request (-1 if not applicable)
const int8_t modemStatusPin = 19;    // MCU pin used to read modem status (-1 if not applicable)
const bool modemStatusLevel = HIGH;  // The level of the status pin when the module is active (HIGH or LOW)
// And create the wake and sleep methods for the modem
// These can be functions of any type and must return a boolean
bool wakeFxn(void)
{
    digitalWrite(modemSleepRqPin, HIGH);
    digitalWrite(redLED, HIGH);  // A light just for show
    return true;
}
bool sleepFxn(void)
{
    digitalWrite(modemSleepRqPin, LOW);
    digitalWrite(redLED, LOW);
    return true;
}

// And we still need the connection information for the network
const char *apn = "xxxxx";  // The APN for the gprs connection, unnecessary for WiFi
const char *wifiId = "xxxxx";  // The WiFi access point, unnecessary for gprs
const char *wifiPwd = "xxxxx";  // The password for connecting to WiFi, unnecessary for gprs

// Create the loggerModem instance
#include <LoggerModem.h>
// A "loggerModem" is a combination of a TinyGSM Modem, a Client, and functions for wake and sleep
loggerModem modem(modemVccPin, modemStatusPin, modemStatusLevel, wakeFxn, sleepFxn, tinyModem, tinyClient, apn);

// Create the RSSI and signal strength variable objects for the modem and return
// variable-type pointers to them
Variable *modemRSSI = new Modem_RSSI(&modem, "12345678-abcd-1234-efgh-1234567890ab");
Variable *modemSignalPct = new Modem_SignalPercent(&modem, "12345678-abcd-1234-efgh-1234567890ab");


// ==========================================================================
//    Maxim DS3231 RTC (Real Time Clock)
// ==========================================================================
#include <sensors/MaximDS3231.h>
// Create and return the DS3231 sensor object
MaximDS3231 ds3231(1);
// Create the temperature variable object for the DS3231 and return a variable-type pointer to it
Variable *ds3231Temp = new MaximDS3231_Temp(&ds3231, "12345678-abcd-1234-efgh-1234567890ab");


#if defined __AVR__
// Set up a serial port for modbus communication - in this case, using AltSoftSerial
#include <AltSoftSerial.h>
AltSoftSerial modbusSerial;
#endif

#if defined ARDUINO_SAMD_ZERO
// On an Arduino Zero or Feather M0, we'll create serial 2 on SERCOM1
#include <wiring_private.h> // Needed for SAMD pinPeripheral() function
Uart Serial2(&sercom1, 11, 10, SERCOM_RX_PAD_0, UART_TX_PAD_2);
// Hand over the interrupts to the sercom port
void SERCOM1_Handler()
{
    Serial2.IrqHandler();
}
HardwareSerial &modbusSerial = Serial2;
#endif

#if defined ARDUINO_SODAQ_AUTONOMO
// Serial2 is already defined on the Autonomo
HardwareSerial &modbusSerial = Serial2;
#endif

// ==========================================================================
//    Yosemitech Y504 Dissolved Oxygen Sensor
// ==========================================================================
#include <sensors/YosemitechY504.h>
byte y504ModbusAddress = 0x04;  // The modbus address of the Y504
const int8_t rs485AdapterPower = 22;  // Pin to switch RS485 adapter power on and off (-1 if unconnected)
const int8_t modbusSensorPower = A3;  // Pin to switch sensor power on and off (-1 if unconnected)
const int8_t max485EnablePin = -1;  // Pin connected to the RE/DE on the 485 chip (-1 if unconnected)
const uint8_t y504NumberReadings = 5;  // The manufacturer recommends averaging 10 readings, but we take 5 to minimize power consumption
// Create and return the Yosemitech Y504 dissolved oxygen sensor object
YosemitechY504 y504(y504ModbusAddress, modbusSerial, rs485AdapterPower, modbusSensorPower, max485EnablePin, y504NumberReadings);
// Create the dissolved oxygen percent, dissolved oxygen concentration, and
// temperature variable objects for the Y504 and return variable-type
// pointers to them
Variable *y504DOpct = new YosemitechY504_DOpct(&y504, "12345678-abcd-1234-efgh-1234567890ab");
Variable *y504DOmgL = new YosemitechY504_DOmgL(&y504, "12345678-abcd-1234-efgh-1234567890ab");
Variable *y504Temp = new YosemitechY504_Temp(&y504, "12345678-abcd-1234-efgh-1234567890ab");


// ==========================================================================
//    Yosemitech Y511 Turbidity Sensor with Wiper
// ==========================================================================
#include <sensors/YosemitechY511.h>
byte y511ModbusAddress = 0x1A;  // The modbus address of the Y511
// const int8_t rs485AdapterPower = 22;  // Pin to switch RS485 adapter power on and off (-1 if unconnected)
// const int8_t modbusSensorPower = A3;  // Pin to switch sensor power on and off (-1 if unconnected)
// const int8_t max485EnablePin = -1;  // Pin connected to the RE/DE on the 485 chip (-1 if unconnected)
const uint8_t y511NumberReadings = 5;  // The manufacturer recommends averaging 10 readings, but we take 5 to minimize power consumption
// Create and return the Y511-A Turbidity sensor object
YosemitechY511 y511(y511ModbusAddress, modbusSerial, rs485AdapterPower, modbusSensorPower, max485EnablePin, y511NumberReadings);
// Create the turbidity and temperature variable objects for the Y511 and return variable-type pointers to them
Variable *y511Turb = new YosemitechY511_Turbidity(&y511, "12345678-abcd-1234-efgh-1234567890ab");
Variable *y511Temp = new YosemitechY511_Temp(&y511, "12345678-abcd-1234-efgh-1234567890ab");


// ==========================================================================
//    Yosemitech Y514 Chlorophyll Sensor
// ==========================================================================
#include <sensors/YosemitechY514.h>
byte y514ModbusAddress = 0x14;  // The modbus address of the Y514
// const int8_t rs485AdapterPower = 22;  // Pin to switch RS485 adapter power on and off (-1 if unconnected)
// const int8_t modbusSensorPower = A3;  // Pin to switch sensor power on and off (-1 if unconnected)
// const int8_t max485EnablePin = -1;  // Pin connected to the RE/DE on the 485 chip (-1 if unconnected)
const uint8_t y514NumberReadings = 5;  // The manufacturer recommends averaging 10 readings, but we take 5 to minimize power consumption
// Create and return the Y514 chlorophyll sensor object
YosemitechY514 y514(y514ModbusAddress, modbusSerial, rs485AdapterPower, modbusSensorPower, max485EnablePin, y514NumberReadings);
// Create the chlorophyll concentration and temperature variable objects for the Y514 and return variable-type pointers to them
Variable *y514Chloro = new YosemitechY514_Chlorophyll(&y514, "12345678-abcd-1234-efgh-1234567890ab");
Variable *y514Temp = new YosemitechY514_Temp(&y514, "12345678-abcd-1234-efgh-1234567890ab");


// ==========================================================================
//    Yosemitech Y520 Conductivity Sensor
// ==========================================================================
#include <sensors/YosemitechY520.h>
byte y520ModbusAddress = 0x20;  // The modbus address of the Y520
// const int8_t rs485AdapterPower = 22;  // Pin to switch RS485 adapter power on and off (-1 if unconnected)
// const int8_t modbusSensorPower = A3;  // Pin to switch sensor power on and off (-1 if unconnected)
// const int8_t max485EnablePin = -1;  // Pin connected to the RE/DE on the 485 chip (-1 if unconnected)
const uint8_t y520NumberReadings = 5;  // The manufacturer recommends averaging 10 readings, but we take 5 to minimize power consumption
// Create and return the Y520 conductivity sensor object
YosemitechY520 y520(y520ModbusAddress, modbusSerial, rs485AdapterPower, modbusSensorPower, max485EnablePin, y520NumberReadings);
// Create the specific conductance and temperature variable objects for the Y520 and return variable-type pointers to them
Variable *y520Cond = new YosemitechY520_Cond(&y520, "12345678-abcd-1234-efgh-1234567890ab");
Variable *y520Temp = new YosemitechY520_Temp(&y520, "12345678-abcd-1234-efgh-1234567890ab");


// ==========================================================================
//    The array that contains all variables to be logged
// ==========================================================================
#include <VariableArray.h>

// Put all of the variable pointers into an Array
// NOTE:  Since we've created all of the variable pointers above, we can just
// reference them by name here.
Variable *variableList_complete[] = {
    mayflySampNo,
    mayflyBatt,
    mayflyRAM,
    ds3231Temp,
    y504DOpct,
    y504DOmgL,
    y504Temp,
    y511Turb,
    y511Temp,
    y514Chloro,
    y514Temp,
    y520Cond,
    y520Temp,
    modemRSSI,
    modemSignalPct
};
// Count up the number of pointers in the array
int variableCount_complete = sizeof(variableList_complete) / sizeof(variableList_complete[0]);
// Create the VariableArray object
VariableArray arrayComplete(variableCount_complete, variableList_complete);

// Create a new logger instance
#include <LoggerBase.h>
Logger loggerAllVars(LoggerID, loggingInterval, sdCardPin, wakePin, &arrayComplete);


// ==========================================================================
//    The array that contains all variables to have their values sent out over the internet
// ==========================================================================

// Put all of the variable pointers into an Array
// NOTE:  Since we've created all of the variable pointers above, we can just
// reference them by name here.
Variable *variableList_toGo[] = {
    y504DOmgL,
    y504Temp,
    y511Turb,
    y514Chloro,
    y520Cond,
    modemRSSI
};
// Count up the number of pointers in the array
int variableCount_toGo = sizeof(variableList_toGo) / sizeof(variableList_toGo[0]);
// Create the VariableArray object
VariableArray arrayToGo(variableCount_toGo, variableList_toGo);
// Create the new logger instance
Logger loggerToGo(LoggerID, loggingInterval,sdCardPin, wakePin, &arrayToGo);


// ==========================================================================
// Device registration and sampling feature information
//   This should be obtained after registration at http://data.envirodiy.org
// ==========================================================================
const char *registrationToken = "12345678-abcd-1234-efgh-1234567890ab";   // Device registration token
const char *samplingFeature = "12345678-abcd-1234-efgh-1234567890ab";     // Sampling feature UUID

// Create a data publisher for the EnviroDIY/WikiWatershed POST endpoint
#include <publishers/EnviroDIYPublisher.h>
EnviroDIYPublisher EnviroDIYPOST(loggerToGo, registrationToken, samplingFeature);


// ==========================================================================
//    Working Functions
// ==========================================================================

// Flashes the LED's on the primary board
void greenredflash(uint8_t numFlash = 4, uint8_t rate = 75)
{
  for (uint8_t i = 0; i < numFlash; i++) {
    digitalWrite(greenLED, HIGH);
    digitalWrite(redLED, LOW);
    delay(rate);
    digitalWrite(greenLED, LOW);
    digitalWrite(redLED, HIGH);
    delay(rate);
  }
  digitalWrite(redLED, LOW);
}


// ==========================================================================
// Main setup function
// ==========================================================================
void setup()
{
    // Start the primary serial connection
    Serial.begin(serialBaud);

    // Print a start-up note to the first serial port
    Serial.print(F("Now running "));
    Serial.print(sketchName);
    Serial.print(F(" on Logger "));
    Serial.println(LoggerID);
    Serial.println();

    Serial.print(F("Using ModularSensors Library version "));
    Serial.println(MODULAR_SENSORS_VERSION);

    if (String(MODULAR_SENSORS_VERSION) !=  String(libraryVersion))
        Serial.println(F(
            "WARNING: THIS EXAMPLE WAS WRITTEN FOR A DIFFERENT VERSION OF MODULAR SENSORS!!"));

    // Start the serial connection with the modem
    ModemSerial.begin(ModemBaud);

    // Start the stream for the modbus sensors
    modbusSerial.begin(9600);

    #if defined ARDUINO_SAMD_ZERO
    // Assign pins to SERCOM functionality
    pinPeripheral(10, PIO_SERCOM);
    pinPeripheral(11, PIO_SERCOM);
    #endif

    // Set up pins for the LED's
    pinMode(greenLED, OUTPUT);
    digitalWrite(greenLED, LOW);
    pinMode(redLED, OUTPUT);
    digitalWrite(redLED, LOW);
    // Blink the LEDs to show the board is on and starting up
    greenredflash();

    // Set up the sleep/wake pin for the modem and put its inital value as "off"
    pinMode(modemSleepRqPin, OUTPUT);
    digitalWrite(modemSleepRqPin, LOW);

    // Set the timezone and offsets
    // Logging in the given time zone
    Logger::setTimeZone(timeZone);
    // Offset is the same as the time zone because the RTC is in UTC
    Logger::setTZOffset(timeZone);

    // Attach the same modem to both loggers
    // It is only needed for the logger that will be sending out data, but
    // attaching it to both allows either logger to control NIST synchronization
    loggerAllVars.attachModem(modem);
    loggerToGo.attachModem(modem);
    loggerAllVars.setTestingModePin(buttonPin);
    // There is no reason to call the setAlertPin() function, because we have to
    // write the loop on our own.

    // Set up the connection information with EnviroDIY for both loggers
    // Doing this for both loggers ensures that the header of the csv will have the tokens in it
    loggerAllVars.setSamplingFeatureUUID(samplingFeature);
    loggerToGo.setSamplingFeatureUUID(samplingFeature);

    // Update the Mayfly "sensor" to get us updated battery voltages
    // We'll use the battery voltage to decide which version of the begin() to use
    // NOTE:  This update happens very fast
    mayfly.update();

    // Because we've given it a modem and it knows all of the tokens, we can
    // just "begin" the complete logger to set up the datafile, clock, sleep,
    // and all of the sensors.  We don't need to bother with the "begin" for the
    // other logger because it has the same processor and clock.
    if (mayflyBatt->getValue() < 3.4) loggerAllVars.begin(true);
    else loggerAllVars.begin();  // set up sensors

    // At very good battery voltage, or with suspicious time stamp, sync the clock
    // Note:  Please change these battery voltages to match your battery
    if (mayflyBatt->getValue() > 3.9 ||
        loggerAllVars.getNowEpoch() < 1545091200 ||  /*Before 12/18/2018*/
        loggerAllVars.getNowEpoch() > 1735689600)  /*Before 1/1/2025*/
        loggerAllVars.syncRTC();
}


// ==========================================================================
// Main loop function
// ==========================================================================

// Because of the way the sleep mode is set up, the processor will wake up
// and start the loop every minute exactly on the minute.
void loop()
{
    // Update the Mayfly "sensor" to get us updated battery voltages
    // NOTE:  This update happens fast enough that it won't throw off the check
    // interval timing.  If the update took more than 1 second to run,
    // it would throw off the checkInterval() function.
    mayfly.update();

    // Assuming we were woken up by the clock, check if the current time is an
    // even interval of the logging interval
    // We're only doing anything at all if the battery is above 3.4V
    if (loggerAllVars.checkInterval() && mayflyBatt->getValue() > 3.4)
    {
        // Print a line to show new reading
        Serial.print(F("------------------------------------------\n"));
        // Turn on the LED to show we're taking a reading
        digitalWrite(greenLED, HIGH);
        digitalWrite(greenLED, HIGH);

        // Turn on the modem to let it start searching for the network
        // Only turn the modem on if the battery at the last interval was high enough
        // NOTE:  if the modemPowerUp function is not run before the completeUpdate
        // function is run, the modem will not be powered and will not return
        // a signal strength readign.
        if (mayflyBatt->getValue() > 3.7)
            modem.modemPowerUp();

        // Start the stream for the modbus sensors
        // Because RS485 adapters tend to "steal" current from the data pins
        // we will explicitly start and end the serial connection in the loop.
        modbusSerial.begin(9600);

        // Do a complete update on the "full" array.
        // This will do all the power management
        // NOTE:  The wake function for each sensor should force sensor setup
        // to run if the sensor was not previously set up.
        Serial.print(F("Updating all sensors...\n"));
        arrayComplete.completeUpdate();

        // End the stream for the modbus sensors
        // Because RS485 adapters tend to "steal" current from the data pins
        // we will explicitly start and end the serial connection in the loop.
        modbusSerial.end();

        #if defined AltSoftSerial_h
        // Explicitly set the pin modes for the AltSoftSerial pins to make sure they're low
        pinMode(5, OUTPUT);  // On a Mayfly, pin D5 is the AltSoftSerial Tx pin
        pinMode(6, OUTPUT);  // On a Mayfly, pin D6 is the AltSoftSerial Rx pin
        digitalWrite(5, LOW);
        digitalWrite(6, LOW);
        #endif

        #if defined ARDUINO_SAMD_ZERO
        digitalWrite(10, LOW);
        digitalWrite(11, LOW);
        #endif

        // Stream the variable results from the complete set of variables to
        // the SD card
        loggerAllVars.logToSD();

        // Connect to the network
        // Again, we're only doing this if the battery is doing well
        if (mayflyBatt->getValue() > 3.7)
        {
            Serial.print(F("Connecting to the internet...\n"));
            if (modem.connectInternet())
            {
                // Post the data to the WebSDL
                loggerToGo.sendDataToRemotes();

                // Disconnect from the network
                modem.disconnectInternet();
            }
            // Turn the modem off
            modem.modemSleepPowerDown();
        }

        // Turn off the LED
        digitalWrite(greenLED, LOW);
        // Print a line to show reading ended
        Serial.print(F("------------------------------------------\n\n"));
    }

    // Check if it was instead the testing interrupt that woke us up
    // Want to enter the testing mode for the "complete" logger so we can see
    // the data from _ALL_ sensors
    // NOTE:  The testingISR attached to the button at the end of the "setup()"
    // function turns on the startTesting flag.  So we know if that flag is set
    // then we want to run the testing mode function.
    if (Logger::startTesting) loggerAllVars.testingMode();

    // Once a day, at midnight, sync the clock
    if (Logger::markedEpochTime % 86400 == 0 && mayflyBatt->getValue() > 3.7)
    {
        // Turn on the modem
        modem.modemPowerUp();
        // Connect to the network
        if (modem.connectInternet())
        {
            // Synchronize the RTC (the loggers have the same clock, pick one)
            loggerAllVars.setRTClock(modem.getNISTTime());
            // Disconnect from the network
            modem.disconnectInternet();
        }
        // Turn off the modem
        modem.modemSleepPowerDown();
    }

    // Call the processor sleep
    // Only need to do this for one of the loggers
    loggerAllVars.systemSleep();
}
