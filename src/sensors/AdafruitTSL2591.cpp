/*
 *AdafruitTSL2591.cpp
 *This file is part of the EnviroDIY modular sensors library for Arduino
 *
 *Initial library developement done by Sara Damiano (sdamiano@stroudcenter.org).
 *Modification of Boschtsl280.cpp by Garrett Warren to support the AdafruitTSL2591
 *
 *This file is for the Adafruit TSL2591 Luminosity Sensor
 *It is dependent on the Adafruit TSL2591 Library
 *
 *Documentation for the sensor can be found at:
 *    https://learn.adafruit.com/adafruit-tsl2591/overview
 *
*/

#include "AdafruitTSL2591.h"


// The constructor - because this is I2C, only need the power pin
AdafruitTSL2591::AdafruitTSL2591(int8_t powerPin, uint8_t i2cAddressHex, uint8_t measurementsToAverage,
  tsl2591Gain_t gain, tsl2591IntegrationTime_t integration)
     : Sensor("AdafruitTSL2591", TSL2591_NUM_VARIABLES,
              TSL2591_WARM_UP_TIME_MS, TSL2591_STABILIZATION_TIME_MS, TSL2591_MEASUREMENT_TIME_MS,
              powerPin, -1, measurementsToAverage)
{
    _i2cAddressHex  = i2cAddressHex;
    _gain = gain;
    _integration = integration;
}
// Destructor
AdafruitTSL2591::~AdafruitTSL2591(){};


String AdafruitTSL2591::getSensorLocation(void)
{
    String address = F("I2C_0x");
    address += String(_i2cAddressHex, HEX);
    return address;
}


bool AdafruitTSL2591::setup(void)
{
    // GNOTE: ADD OVERSAMPLING SETUP

    bool retVal = Sensor::setup();  // this will set pin modes and the setup status bit

    // This sensor needs power for setup!
    // The TSL2591's begin() reads required calibration data from the sensor.
    bool wasOn = checkPowerOn();
    if (!wasOn) {powerUp();}
    waitForWarmUp();

    // Run begin fxn because it returns true or false for success in contact
    // Make 5 attempts
    uint8_t ntries = 0;
    bool success = false;
    while (!success and ntries < 5)
    {
        success = tsl_internal.begin();
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

    //set gain and timing
    tsl_internal.setGain(_gain);
    tsl_internal.setTiming(_integration);


    return retVal;
}


bool AdafruitTSL2591::wake(void)
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


bool AdafruitTSL2591::addSingleMeasurementResult(void)
{
    bool success = false;

    // declare measurement variables
    uint32_t lum;
    float full, ir, vis, lux;

    // Check a measurement was *successfully* started (status bit 6 set)
    // Only go on to get a result if it was
    if (bitRead(_sensorStatus, 6))
    {
        MS_DBG(getSensorNameAndLocation(), F("is reporting:"));
        lum = tsl_internal.getFullLuminosity();
        ir = (float)(lum >> 16);
        full = (float)(lum & 0xFFFF);
        vis = full - ir;

        if (isnan(ir) || (ir == 0)) ir = -9999;
        if (isnan(full) || (full == 0)) full = -9999;
        if (isnan(vis) || (vis == 0)) vis = -9999;


        if ((ir!=-9999) && (full!=-9999)) {
          lux = tsl_internal.calculateLux(ir, full);
        } else lux = -9999;


        // Assume that if all three are 0, really a failed response
        // May also return a very negative temp when receiving a bad response
        if ((ir == 0 && full == 0 && vis == 0 && lux == 0))
        {
            MS_DBG(F("All values 0 or bad, assuming sensor non-response!"));
            ir = -9999;
            full = -9999;
            vis = -9999;
            lux = -9999;
        }
        else success = true;

        MS_DBG(F("  Full Spectrum:"), full, F("raw ADC"));
        MS_DBG(F("  Infrared:"), ir, F("raw ADC"));
        MS_DBG(F("  Visible:"), vis, F("raw ADC"));
        MS_DBG(F("  Illuminance:"), lux, F("Lux"));
    }
    else MS_DBG(getSensorNameAndLocation(), F("is not currently measuring!"));


    verifyAndAddMeasurementResult(TSL2591_FULLSPECTRUM_VAR_NUM, full);
    verifyAndAddMeasurementResult(TSL2591_INFRARED_RESOLUTION, ir);
    verifyAndAddMeasurementResult(TSL2591_VISIBLE_VAR_NUM, vis);
    verifyAndAddMeasurementResult(TSL2591_ILLUMINANCE_VAR_NUM, lux);

    // Unset the time stamp for the beginning of this measurement
    _millisMeasurementRequested = 0;
    // Unset the status bits for a measurement request (bits 5 & 6)
    _sensorStatus &= 0b10011111;

    return success;
}
