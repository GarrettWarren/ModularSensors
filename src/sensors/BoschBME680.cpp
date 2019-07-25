/*
 *BoschBME680.cpp
 *This file is part of the EnviroDIY modular sensors library for Arduino
 *
 *Initial library developement done by Sara Damiano (sdamiano@stroudcenter.org).
 *Modification of BoschBME280.cpp by Garrett Warren to support the BoschBME680
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
 * Slowest response time (humidity): 1sec
*/

#include "BoschBME680.h"


// The constructor - because this is I2C, only need the power pin
BoschBME680::BoschBME680(int8_t powerPin, uint8_t i2cAddressHex, uint8_t measurementsToAverage)
     : Sensor("BoschBME680", BME680_NUM_VARIABLES,
              BME680_WARM_UP_TIME_MS, BME680_STABILIZATION_TIME_MS, BME680_MEASUREMENT_TIME_MS,
              powerPin, -1, measurementsToAverage)
{
    _i2cAddressHex  = i2cAddressHex;
}
// Destructor
BoschBME680::~BoschBME680(){};


String BoschBME680::getSensorLocation(void)
{
    String address = F("I2C_0x");
    address += String(_i2cAddressHex, HEX);
    return address;
}


bool BoschBME680::setup(void)
{
    // GNOTE: ADD OVERSAMPLING SETUP

    bool retVal = Sensor::setup();  // this will set pin modes and the setup status bit

    // This sensor needs power for setup!
    // The bme680's begin() reads required calibration data from the sensor.
    bool wasOn = checkPowerOn();
    if (!wasOn) {powerUp();}
    waitForWarmUp();

    // Run begin fxn because it returns true or false for success in contact
    // Make 5 attempts
    uint8_t ntries = 0;
    bool success = false;
    while (!success and ntries < 5)
    {
        success = bme_internal.begin(_i2cAddressHex);
        ntries++;
    }
    if (!success)
    {
        // Set the status error bit (bit 7)
        _sensorStatus |= 0b10000000;
        // UN-set the set-up bit (bit 0) since setup failed!
        _sensorStatus &= 0b11111110;
    }
    retVal &= success;

    // Turn the power back off it it had been turned on
    if (!wasOn) {powerDown();}

    // Set up oversampling and filter initialization
    bme_internal.setTemperatureOversampling(BME680_OS_8X);
    bme_internal.setHumidityOversampling(BME680_OS_2X);
    bme_internal.setPressureOversampling(BME680_OS_4X);
    bme_internal.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme_internal.setGasHeater(320, 150); // 320*C for 150 ms

    return retVal;
}


bool BoschBME680::wake(void)
{
    // Sensor::wake() checks if the power pin is on, setup has been successful,
    // and sets the wake timestamp and status bits.  If it returns false,
    // there's no reason to go on.
    if (!Sensor::wake()) return false;

    // Restart always needed after power-up to set sampling modes
    // As of Adafruit library version 1.0.7, this function includes all of the
    // various delays to allow the chip to wake up, get calibrations, get
    // coefficients, and set sampling modes.
    // This will also restart "Wire"
    // Currently this is using the settings that Adafruit considered to be 'default'

    delay(100);//G NOTE: TEST IF NEEDED
    return true;
}


bool BoschBME680::addSingleMeasurementResult(void)
{
    bool success = false;

    // Initialize float variables
    float temp = -9999;
    float humid = -9999;
    float press = -9999;
    float gas = -9999;

    // Check a measurement was *successfully* started (status bit 6 set)
    // Only go on to get a result if it was

    if (bitRead(_sensorStatus, 6))
    {
        MS_DBG(getSensorNameAndLocation(), F("is reporting:"));
        bme_internal.performReading(); // read values
        temp = bme_internal.temperature;
        if (isnan(temp)) temp = -9999;
        humid = bme_internal.humidity;
        if (isnan(humid)) humid = -9999;
        press = bme_internal.pressure;
        if (isnan(press)) press = -9999;
        gas = bme_internal.gas_resistance;
        if (isnan(gas)) gas = -9999;

        // Assume that if all three are 0, really a failed response
        // May also return a very negative temp when receiving a bad response
        else if ((temp == 0 && press == 0 && humid == 0 && gas == 0) || temp < -40)
        {
            MS_DBG(F("All values 0 or bad, assuming sensor non-response!"));
            temp =  -9999;
            press = -9999;
            humid = -9999;
            gas = -9999;
        }
        else success = true;

        MS_DBG(F("  Temperature:"), temp, F("°C"));
        MS_DBG(F("  Humidity:"), humid, F("%RH"));
        MS_DBG(F("  Barometric Pressure:"), press, F("Pa"));
        MS_DBG(F("  Gas Resistance:"), gas, F("Ohm"));
    }
    else MS_DBG(getSensorNameAndLocation(), F("is not currently measuring!"));

    verifyAndAddMeasurementResult(BME680_TEMP_VAR_NUM, temp);
    verifyAndAddMeasurementResult(BME680_HUMIDITY_VAR_NUM, humid);
    verifyAndAddMeasurementResult(BME680_PRESSURE_VAR_NUM, press);
    verifyAndAddMeasurementResult(BME680_GAS_VAR_NUM, gas);

    // Unset the time stamp for the beginning of this measurement
    _millisMeasurementRequested = 0;
    // Unset the status bits for a measurement request (bits 5 & 6)
    _sensorStatus &= 0b10011111;

    return success;
}
