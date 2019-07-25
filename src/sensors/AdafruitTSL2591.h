/*
 *AdafruitTSL2591.h
 *This file is part of the EnviroDIY modular sensors library for Arduino
 *
 *Initial library developement done by Sara Damiano (sdamiano@stroudcenter.org).
 *This file is by Garrett Warren to support the TSL2591. The BoschBME280.h was
 *used as a starter template.
 *
 *This file is for the Adafruit TSL2591 Light Sensor
 *It is dependent on the Adafruit TSL2591 Library
 *
 *Documentation for the sensor can be found at:
 *https://www.adafruit.com/product/1980
 *The datasheet can be found in the docs directory
 *
 * The sensor is capable of sensing light intensity in both the broadband and IR
 * light spectrums. From these measurements, the visible light spectrum
 * intensity can also be found. Additionally, these values are combined using a
 * library method that uses an emperical formula to compute the total
 * illuminance (ambient light level), measured in lux.
 *
 * There are two parameters that can be adjusted to modify the sensor outputs
 * for different environments. Change the protected variable in this file.
 *    1. Analog Gain: Use a higher gain for low-light conditions
 *      Options (from the AdafruitTSL2591 library):
 *                TSL2591_GAIN_LOW = 0x00,     low gain (1x)
 *                TSL2591_GAIN_MED = 0x10,     medium gain (25x)
 *                TSL2591_GAIN_HIGH= 0x20,     medium gain (428x)
 *                TSL2591_GAIN_MAX = 0x30,     max gain (9876x)
 *
 *    2. Integration Time: Use a longer integration time for low-light conditions
 *      Options (from the AdafruitTSL2591 library):
 *                TSL2591_INTEGRATIONTIME_100MS     = 0x00,   100 millis
 *                TSL2591_INTEGRATIONTIME_200MS     = 0x01,   200 millis
 *                TSL2591_INTEGRATIONTIME_300MS     = 0x02,   300 millis
 *                TSL2591_INTEGRATIONTIME_400MS     = 0x03,   400 millis
 *                TSL2591_INTEGRATIONTIME_500MS     = 0x04,   500 millis
 *                TSL2591_INTEGRATIONTIME_600MS     = 0x05,   600 millis
 *
 * Power Consumption:
 *  0.4 mA when active
 *  3 uA sleep
 *
 *
 * Variable Info:
 *
 *    FullSpectrum:
 *      - the value from the broadband photodiode (visible plus infrared)
 *      - units: raw ADC
 *
 *    Infrared:
 *      - the value from the infrared-responding photodio
 *      - units: raw ADC
 *
 *    Visible:
 *      - the difference between the FullSpectrum and Infrared measurements
 *      - units: raw ADC
 *
 *    Illuminance:
 *      - the radiometric measurements, the irradiance (W/m^2), from the sensors
 *        is emperically converted to a photometric measurment of illuminance
 *        (lux); however, the accuracy of this conversion is disputed and
 *        explained in this thread:
 *        https://github.com/adafruit/Adafruit_TSL2591_Library/issues/14
 *      - units: lux
 *      - range: 188 uLux - 88 kLux
 *
 *
 * Sensor response time is between 100-600 ms (depending on integration time)
*/

// Header Guards
#ifndef AdafruitTSL2591_h
#define AdafruitTSL2591_h

// Debugging Statement
#define MS_ADAFRUITTSL2591_DEBUG

#ifdef MS_ADAFRUITTSL2591_DEBUG
#define MS_ADAFRUITTSL2591_STD "AdafruitTSL2591"
#endif

// Included Dependencies
#include "ModSensorDebugger.h"
#include "VariableBase.h"
#include "SensorBase.h"
#include <Adafruit_TSL2591.h>

// Sensor Specific Defines
#define TSL2591_NUM_VARIABLES 4
#define TSL2591_WARM_UP_TIME_MS 100                                             //TODO: TEST
#define TSL2591_STABILIZATION_TIME_MS 100                                       // TODO experiment with this value
#define TSL2591_MEASUREMENT_TIME_MS 600


#define TSL2591_ILLUMINANCE_RESOLUTION 1                                        //TODO: WHAT TO DO ABOUT RESONLUTIONS FOR THESE VARS?
#define TSL2591_ILLUMINANCE_VAR_NUM 1

#define TSL2591_INFRARED_RESOLUTION 0
#define TSL2591_INFRARED_VAR_NUM 1

#define TSL2591_VISIBLE_RESOLUTION 0
#define TSL2591_VISIBLE_VAR_NUM 2

#define TSL2591_FULLSPECTRUM_RESOLUTION 0
#define TSL2591_FULLSPECTRUM_VAR_NUM 3


// The main class for the Bosch TSL2591
class AdafruitTSL2591 : public Sensor
{
public:
    AdafruitTSL2591(int8_t powerPin, uint8_t i2cAddressHex = 0x29,
      uint8_t measurementsToAverage = 1,                                        //T0D0 TEST MEASUREMENT AVGING?
      tsl2591Gain_t gain = TSL2591_GAIN_LOW,
      tsl2591IntegrationTime_t integration = TSL2591_INTEGRATIONTIME_100MS);
    ~AdafruitTSL2591();

    bool wake(void) override;
    bool setup(void) override;
    String getSensorLocation(void) override;

    // bool startSingleMeasurement(void) override;  // for forced mode
    bool addSingleMeasurementResult(void) override;

protected:
    Adafruit_TSL2591 tsl_internal;
    uint8_t _i2cAddressHex;
    // user programmable parameters:
    tsl2591Gain_t _gain;
    tsl2591IntegrationTime_t _integration;
};


// Defines the Infrared Variable
class AdafruitTSL2591_Infrared : public Variable
{
public:
    AdafruitTSL2591_Infrared(Sensor *parentSense,
                     const char *uuid = "",
                     const char *varCode = "AdafruitTSL2591_Infrared")
      : Variable(parentSense,
                 (const uint8_t)TSL2591_INFRARED_VAR_NUM,
                 (uint8_t)TSL2591_INFRARED_RESOLUTION,
                 "Infrared photodiode reading", "raw ADC",
                 varCode, uuid)
    {}
    AdafruitTSL2591_Infrared()
      : Variable((const uint8_t)TSL2591_INFRARED_VAR_NUM,
                 (uint8_t)TSL2591_INFRARED_RESOLUTION,
                 "Infrared photodiode reading", "raw ADC", "AdafruitTSL2591_Infrared")
    {}
    ~AdafruitTSL2591_Infrared(){};
};


// Defines the Visible Light Variable
class AdafruitTSL2591_Visible : public Variable
{
public:
    AdafruitTSL2591_Visible(Sensor *parentSense,
                     const char *uuid = "",
                     const char *varCode = "AdafruitTSL2591_Visible")
      : Variable(parentSense,
                 (const uint8_t)TSL2591_VISIBLE_VAR_NUM,
                 (uint8_t)TSL2591_VISIBLE_RESOLUTION,
                 "Visible light photodiode reading", "raw ADC",
                 varCode, uuid)
    {}
    AdafruitTSL2591_Visible()
      : Variable((const uint8_t)TSL2591_VISIBLE_VAR_NUM,
                 (uint8_t)TSL2591_VISIBLE_RESOLUTION,
                 "Visible light photodiode reading", "raw ADC", "AdafruitTSL2591_Visible")
    {}
    ~AdafruitTSL2591_Visible(){};
};


// Defines the FullSpectrum Variable
class AdafruitTSL2591_FullSpectrum : public Variable
{
public:
    AdafruitTSL2591_FullSpectrum(Sensor *parentSense,
                     const char *uuid = "",
                     const char *varCode = "AdafruitTSL2591_FullSpectrum")
      : Variable(parentSense,
                 (const uint8_t)TSL2591_FULLSPECTRUM_VAR_NUM,
                 (uint8_t)TSL2591_FULLSPECTRUM_RESOLUTION,
                 "Full Spectrum photodiode reading", "raw ADC",
                 varCode, uuid)
    {}
    AdafruitTSL2591_FullSpectrum()
      : Variable((const uint8_t)TSL2591_FULLSPECTRUM_VAR_NUM,
                 (uint8_t)TSL2591_FULLSPECTRUM_RESOLUTION,
                 "Full Spectrum photodiode", "raw ADC", "AdafruitTSL2591_FullSpectrum")
    {}
    ~AdafruitTSL2591_FullSpectrum(){};
};


// Defines the Illuminance Variable
class AdafruitTSL2591_Illuminance : public Variable
{
public:
    AdafruitTSL2591_Illuminance(Sensor *parentSense,
                     const char *uuid = "",
                     const char *varCode = "AdafruitTSL2591_Illuminance")
      : Variable(parentSense,
                 (const uint8_t)TSL2591_ILLUMINANCE_VAR_NUM,
                 (uint8_t)TSL2591_ILLUMINANCE_RESOLUTION,
                 "Illuminance", "lux",
                 varCode, uuid)
    {}
    AdafruitTSL2591_Illuminance()
      : Variable((const uint8_t)TSL2591_FULLSPECTRUM_VAR_NUM,
                 (uint8_t)TSL2591_FULLSPECTRUM_RESOLUTION,
                 "Illuminance", "lux", "AdafruitTSL2591_Illuminance")
    {}
    ~AdafruitTSL2591_Illuminance(){};
};


#endif  // Header Guard
