/*
 *MeterGroupTerros12.h
 *This file is part of the EnviroDIY modular sensors library for Arduino
 *
 *Initial library developement done by Sara Damiano (sdamiano@stroudcenter.org).
 *This file created for MeterGroupGroup Terros12 sensor by Garrett Warren from a
 *modification of the Decagon5TM.h file
 *
 *This file is for the MeterGroup Terros 12
 *It is dependent on the EnviroDIY SDI-12 library and the SDI12Sensors super class.
 *
 *Documentation for the SDI-12 Protocol commands and responses
 *for the MeterGroup can be found at:
 * http://publications.metergroup.com/Integrator%20Guide/18224%20TEROS%2011-12%20Integrator%20Guide.pdf
 *
 * For Volumetric Water Content (VWC):
 *     Resolution is 0.001 m3/m3
 *     Accuracy for Generic calibration equation: ± 0.03 m3/m3 (± 3% VWC) typ
 *     Accuracy for Medium Specific Calibration: ± 0.01-0.02 m3/m3 (± 1-2% VWC)
 *     Range is 0 – 1 m3/m3 (soiless media) and 0 - 0.7 m3/m3 (mineral soil)
 *     Apparent dielectric permitivity can range from 1 (air) to 80 (water)
 *
 * For Temperature:
 *     Resolution is 0.1°C
 *     Accuracy is ± 0.5°C from - 40°C to 0°C and ± 0.3°C from 0°C to 60°C
 *     Range is - 40°C to + 60°C
 *
 * For Bulk Electrical Conductivity (EC)
 *     Resolution 0.001 dS/m
 *     Accuracy is ±(5% + 0.01 dS/m) from 0–10 dS/m and ±8% from 10–20 dS/m
 *     Range is 0–20 dS/m (bulk)
 *
 * Typical power up time in SDI-12 mode: 245ms, assume stability at warm-up
 * Maximum measurement duration: 50ms
 *
 * Current drain:
 *     sleep: 0.03 mA
 *     measurment (25 ms): min = 3.0 mA  max = 16.0 mA  typical = 3.6 mA
*/

// Header Guards
#ifndef MeterGroupTerros12_h
#define MeterGroupTerros12_h

// Debugging Statement
// #define MS_MeterGroupTerros12_DEBUG

#ifdef MS_MeterGroupTerros12_DEBUG
#define MS_DEBUGGING_STD "MeterGroupTerros12"
#endif

// Included Dependencies
#include "ModSensorDebugger.h"
#undef MS_DEBUGGING_STD
#include "VariableBase.h"
#include "sensors/SDI12Sensors.h"

// Sensor Specific Defines
#define Terros12_NUM_VARIABLES 3
#define Terros12_WARM_UP_TIME_MS 245
#define Terros12_STABILIZATION_TIME_MS 0
#define Terros12_MEASUREMENT_TIME_MS 50

#define Terros12_VWC_RESOLUTION 3
#define Terros12_VWC_VAR_NUM 0

#define Terros12_TEMP_RESOLUTION 1
#define Terros12_TEMP_VAR_NUM 1

#define Terros12_EC_RESOLUTION 3
// adding extra digit to resolution for averaging
#define Terros12_EC_VAR_NUM 2

// The main class for the MeterGroup Terros12
class MeterGroupTerros12 : public SDI12Sensors
{
public:
    // Constructors with overloads
    MeterGroupTerros12(char SDI12address, int8_t powerPin, int8_t dataPin, uint8_t measurementsToAverage = 1)
     : SDI12Sensors(SDI12address, powerPin, dataPin, measurementsToAverage,
                    "MeterGroupTerros12", Terros12_NUM_VARIABLES,
                    Terros12_WARM_UP_TIME_MS, Terros12_STABILIZATION_TIME_MS, Terros12_MEASUREMENT_TIME_MS)
    {}
    MeterGroupTerros12(char *SDI12address, int8_t powerPin, int8_t dataPin, uint8_t measurementsToAverage = 1)
     : SDI12Sensors(SDI12address, powerPin, dataPin, measurementsToAverage,
                    "MeterGroupTerros12", Terros12_NUM_VARIABLES,
                    Terros12_WARM_UP_TIME_MS, Terros12_STABILIZATION_TIME_MS, Terros12_MEASUREMENT_TIME_MS)
    {}
    MeterGroupTerros12(int SDI12address, int8_t powerPin, int8_t dataPin, uint8_t measurementsToAverage = 1)
     : SDI12Sensors(SDI12address, powerPin, dataPin, measurementsToAverage,
                    "MeterGroupTerros12", Terros12_NUM_VARIABLES,
                    Terros12_WARM_UP_TIME_MS, Terros12_STABILIZATION_TIME_MS, Terros12_MEASUREMENT_TIME_MS)
    {}
    // Destructor
    ~MeterGroupTerros12(){}

    virtual bool addSingleMeasurementResult(void) override;
};


// Defines the Volumetric Water Content Variable
class MeterGroupTerros12_VWC : public Variable
{
public:
    MeterGroupTerros12_VWC(Sensor *parentSense,
                   const char *uuid = "",
                   const char *varCode = "SoilVWC")
      : Variable(parentSense,
                 (const uint8_t)Terros12_VWC_VAR_NUM,
                 (uint8_t)Terros12_VWC_RESOLUTION,
                 "volumetricWaterContent", "percent",
                 varCode, uuid)
    {}
    MeterGroupTerros12_VWC()
      : Variable((const uint8_t)Terros12_VWC_VAR_NUM,
                 (uint8_t)Terros12_VWC_RESOLUTION,
                 "volumetricWaterContent", "percent", "SoilVWC")
    {}
    ~MeterGroupTerros12_VWC(){}
};


// Defines the Temperature Variable
class MeterGroupTerros12_Temp : public Variable
{
public:
    MeterGroupTerros12_Temp(Sensor *parentSense,
                    const char *uuid = "",
                    const char *varCode = "SoilTemp")
      : Variable(parentSense,
                 (const uint8_t)Terros12_TEMP_VAR_NUM,
                 (uint8_t)Terros12_TEMP_RESOLUTION,
                 "temperature", "degreeCelsius",
                 varCode, uuid)
    {}
    MeterGroupTerros12_Temp()
      : Variable((const uint8_t)Terros12_TEMP_VAR_NUM,
                 (uint8_t)Terros12_TEMP_RESOLUTION,
                 "temperature", "degreeCelsius", "SoilTemp")
    {}
    ~MeterGroupTerros12_Temp(){}
};


// Defines the Bulk Electrical Conductivity Variable
class MeterGroupTerros12_EC : public Variable
{
public:
    MeterGroupTerros12_EC(Sensor *parentSense,
                    const char *uuid = "",
                    const char *varCode = "SoilEC")
      : Variable(parentSense,
                 (const uint8_t)Terros12_EC_VAR_NUM,
                 (uint8_t)Terros12_EC_RESOLUTION,
                 "bulkElectricalConductivity", "dS/m",
                 varCode, uuid)
    {}
    MeterGroupTerros12_EC()
      : Variable((const uint8_t)Terros12_EC_VAR_NUM,
                 (uint8_t)Terros12_EC_RESOLUTION,
                 "bulkElectricalConductivity", "dS/m", "SoilEC")
    {}
    ~MeterGroupTerros12_EC(){}
};




#endif  // Header Guard
