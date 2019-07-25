/*
 *TODO: FILL IN
 *
 */

#include "MeterGroupTerros12.h"

bool MeterGroupTerros12::addSingleMeasurementResult(void)
{
    bool success = false;

    // Set up the float variables for receiving data
    float vwc = -9999;
    float temp = -9999;
    float ec = -9999;

    // Check a measurement was *successfully* started (status bit 6 set)
    // Only go on to get a result if it was
    if (bitRead(_sensorStatus, 6))
    {
        // MS_DBG(F("   Activating SDI-12 instance for"), getSensorNameAndLocation());
        // Check if this the currently active SDI-12 Object
        bool wasActive = _SDI12Internal.isActive();
        // if (wasActive) {MS_DBG(F("   SDI-12 instance for"), getSensorNameAndLocation(),
        //                       F("was already active!"));}
        // If it wasn't active, activate it now.
        // Use begin() instead of just setActive() to ensure timer is set correctly.
        if (!wasActive) _SDI12Internal.begin();
        // Empty the buffer
        _SDI12Internal.clearBuffer();


        /*
          SEND THE MEASUREMENT COMMAND
          SDI-12 measurement command format  [address]['M'][!]
         */
         // Use this to take a measurement from a single sensor on the SDI-12 bus
         // if you have multiple sensors on the bus, refer to the concurrent
         // measurment command in the integrators guide linked at the top of this file
         String measurementCommand = "";
         measurementCommand += _SDI12address;
         measurementCommand += "M!";
         Serial.print("meascommand: ");
         Serial.println(measurementCommand);
         _SDI12Internal.sendCommand(measurementCommand);


        /*
          RECEIVE THE MEASUREMENT RESPONSE:
          wait for acknowlegement with format
          [address][ttt (3 char, seconds)][number of measurments available, 0-9]
         */
         delay(30);
         String measurementResponse = "";
         while (_SDI12Internal.available())  // build response string
         {
           char c = _SDI12Internal.read();
           if ((c != '\n') && (c != '\r'))
           {
             measurementResponse += c;
             delay(5);
           }
         }
         _SDI12Internal.clearBuffer();

         // find out how long we have to wait (in seconds).
         uint8_t wait = 0;
         wait = measurementResponse.substring(1,4).toInt();

         unsigned long timerStart = millis();
         while((millis() - timerStart) < (1000 * wait)){
           if(_SDI12Internal.available())  // sensor can interrupt us to let us know it is done early
           {
             _SDI12Internal.clearBuffer();
             break;
           }
         }
         // Wait for anything else and clear it out
         delay(30);
         _SDI12Internal.clearBuffer();


        /*
          SEND THE DATA COMMAND
         */
        // SDI-12 command to get data [address][D][dataOption][!]
        String dataCommand = "";
        dataCommand += _SDI12address;
        dataCommand += "D0!";
        Serial.print("datacommand: ");
        Serial.println(dataCommand);
        _SDI12Internal.sendCommand(dataCommand);

        /*
          RECEIVE THE DATA RESPONSE
          wait for data with format
          address+<calibratedCountsVWC>±<temperature>+<electricalConductivity>
          use the SDI-12 library's parseFloat() function to extract float values
         */
        while(!_SDI12Internal.available()>1); // wait for acknowlegement
        delay(300); // let the data transfer

        String buffer = "";
        _SDI12Internal.read(); // consume address

        // First variable returned is the volumetric water content in %
        vwc = _SDI12Internal.parseFloat();
        Serial.print("vwc: ");
        Serial.println(vwc);
        if (vwc < 0 || vwc > 1000) vwc = -9999;
        // Second variable returned is the temperature in °C
        temp = _SDI12Internal.parseFloat();
        Serial.print("temp: ");
        Serial.println(temp);
        if (temp < -40 || temp > 60) temp = -9999;  // Range is - 40°C to + 60°C

        // Third variable returned is the bulk electrical conductivity in dS/m
        ec = _SDI12Internal.parseFloat();
        Serial.print("ec: ");
        Serial.println(ec);
        if (ec < 0) ec = -9999;

        MS_DBG(getSensorNameAndLocation(), F("is reporting:"));

        uint32_t start = millis();
        while (_SDI12Internal.available() < 3 && (millis() - start) < 1500) {}
        MS_DBG(F("  Receiving results from"), getSensorNameAndLocation());

        // Empty the buffer again
        _SDI12Internal.clearBuffer();

        // De-activate the SDI-12 Object
        // Use end() instead of just forceHold to un-set the timers
        _SDI12Internal.end();

        MS_DBG(F("  Volumetric Water Content:"), vwc);
        MS_DBG(F("  Temperature:"), temp);
        MS_DBG(F("  Bulk Electrical Conductivity E:"), ec);

        success = true;
    }
    else
    {
        MS_DBG(getSensorNameAndLocation(), F("is not currently measuring!"));
    }

    verifyAndAddMeasurementResult(Terros12_VWC_VAR_NUM, vwc);
    verifyAndAddMeasurementResult(Terros12_TEMP_VAR_NUM, temp);
    verifyAndAddMeasurementResult(Terros12_EC_VAR_NUM, ec);

    // Unset the time stamp for the beginning of this measurement
    _millisMeasurementRequested = 0;
    // Unset the status bits for a measurement request (bits 5 & 6)
    _sensorStatus &= 0b10011111;

    return success;
}
