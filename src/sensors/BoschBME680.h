/*
 *BoschBME680.h
 *This file is part of the EnviroDIY modular sensors library for Arduino
 *
 *Initial library developement done by Sara Damiano (sdamiano@stroudcenter.org).
 *Modification of BoschBME280.h by Garrett Warren to support the BoschBME680
 *
 *This file is for the Bosch BME680 Digital Pressure and Humidity Sensor
 *It is dependent on the Adafruit BME680 Library
 *
 *Documentation for the sensor can be found at:
 *https://www.bosch-sensortec.com/bst/products/all_products/bme680
 *
 * For Barometric Pressure:
 *  Resolution is 0.18Pa
 *  Absolute Accuracy is ±0.6hPa
 *  Relative Accuracy is ±0.12hPa
 *  Range is 300 to 1100 hPa
 *
 * For Temperature:
 *  Resolution is 0.01°C
 *  Accuracy is ±0.5°C at 25°C, ±1.0°C from 0-65°C
 *  Range is -40°C to +85°C
 *
 * For Humidity:
 *  Resolution is 0.008 % RH (16 bit)
 *  Accuracy is ± 3 % RH
 *  Range is 0 to 100 % RH, Full accuracy between 10 to 90 % RH
 *
 * For Gas:
 *  Resolution is 0.08
 *
 * Note: Altitude can be added as a calculated variable. It is best not to add
 * it as a sensor variable because calling readAltitude causes the sensor
 * to take another full set of measurements.
 *
 * Sensor takes about 100ms to respond
 * Slowest response time (humidity): 8sec
 * Assume sensor is immediately stable
*/

// Header Guards
#ifndef BoschBME680_h
#define BoschBME680_h

// Debugging Statement
#define MS_BOSCHBME680_DEBUG

#ifdef MS_BOSCHBME680_DEBUG
#define MS_DEBUGGING_STD
#endif

// Included Dependencies
#include "ModSensorDebugger.h"
#include "VariableBase.h"
#include "SensorBase.h"
#include <Adafruit_BME680.h>

// Sensor Specific Defines
#define BME680_NUM_VARIABLES 4
#define BME680_WARM_UP_TIME_MS 100
#define BME680_STABILIZATION_TIME_MS 1000   // TODO experiment with this value
#define BME680_MEASUREMENT_TIME_MS 1100     // TODO experiment as done below:

#define BME680_TEMP_RESOLUTION 2
#define BME680_TEMP_VAR_NUM 0

#define BME680_HUMIDITY_RESOLUTION 3
#define BME680_HUMIDITY_VAR_NUM 1

#define BME680_PRESSURE_RESOLUTION 2
#define BME680_PRESSURE_VAR_NUM 2

#define BME680_GAS_RESOLUTION 2
#define BME680_GAS_VAR_NUM 3

// The main class for the Bosch BME680
class BoschBME680 : public Sensor
{
public:
    BoschBME680(int8_t powerPin, uint8_t i2cAddressHex = 0x77, uint8_t measurementsToAverage = 1); //T0D0 TEST MEASUREMENT AVGING?
    ~BoschBME680();

    bool wake(void) override;
    bool setup(void) override;
    String getSensorLocation(void) override;

    // bool startSingleMeasurement(void) override;  // for forced mode
    bool addSingleMeasurementResult(void) override;

protected:
    Adafruit_BME680 bme_internal;
    uint8_t _i2cAddressHex;
};


// Defines the Temperature Variable
class BoschBME680_Temp : public Variable
{
public:
    BoschBME680_Temp(Sensor *parentSense,
                     const char *uuid = "",
                     const char *varCode = "BoschBME680Temp")
      : Variable(parentSense,
                 (const uint8_t)BME680_TEMP_VAR_NUM,
                 (uint8_t)BME680_TEMP_RESOLUTION,
                 "temperature", "degreeCelsius",
                 varCode, uuid)
    {}
    BoschBME680_Temp()
      : Variable((const uint8_t)BME680_TEMP_VAR_NUM,
                 (uint8_t)BME680_TEMP_RESOLUTION,
                 "temperature", "degreeCelsius", "BoschBME680Temp")
    {}
    ~BoschBME680_Temp(){};
};


// Defines the Humidity Variable
class BoschBME680_Humidity : public Variable
{
public:
    BoschBME680_Humidity(Sensor *parentSense,
                         const char *uuid = "",
                         const char *varCode = "BoschBME680Humidity")
      : Variable(parentSense,
                 (const uint8_t)BME680_HUMIDITY_VAR_NUM,
                 (uint8_t)BME680_HUMIDITY_RESOLUTION,
                 "relativeHumidity", "percent",
                 varCode, uuid)
    {}
    BoschBME680_Humidity()
      : Variable((const uint8_t)BME680_HUMIDITY_VAR_NUM,
                 (uint8_t)BME680_HUMIDITY_RESOLUTION,
                 "relativeHumidity", "percent", "BoschBME680Humidity")
    {}
    ~BoschBME680_Humidity(){};
};


// Defines the Pressure Variable
class BoschBME680_Pressure : public Variable
{
public:
    BoschBME680_Pressure(Sensor *parentSense,
                         const char *uuid = "",
                         const char *varCode = "BoschBME680Pressure")
      : Variable(parentSense,
                 (const uint8_t)BME680_PRESSURE_VAR_NUM,
                 (uint8_t)BME680_PRESSURE_RESOLUTION,
                 "barometricPressure", "pascal",
                 varCode, uuid)
    {}
    BoschBME680_Pressure()
      : Variable((const uint8_t)BME680_PRESSURE_VAR_NUM,
                 (uint8_t)BME680_PRESSURE_RESOLUTION,
                 "barometricPressure", "pascal", "BoschBME680Pressure")
    {}
};


// Defines the Gas Variable
class BoschBME680_Gas : public Variable
{
public:
    BoschBME680_Gas(Sensor *parentSense,
                         const char *uuid = "",
                         const char *varCode = "BoschBME680Gas")
      : Variable(parentSense,
                 (const uint8_t)BME680_GAS_VAR_NUM,
                 (uint8_t)BME680_GAS_RESOLUTION,
                 "gasResistance", "Ohm",
                 varCode, uuid)
    {}
    BoschBME680_Gas()
      : Variable((const uint8_t)BME680_GAS_VAR_NUM,
                 (uint8_t)BME680_GAS_RESOLUTION,
                 "gasResistance", "Ohm", "BoschBME680Gas")
    {}
};


#endif  // Header Guard
