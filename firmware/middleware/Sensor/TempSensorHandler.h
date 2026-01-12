#ifndef TEMP_SENSOR_HANDLER_H
#define TEMP_SENSOR_HANDLER_H

#include "TempSensorDriver.h"

class TempSensorHandler {
public:
    TempSensorHandler(uint8_t pin);
    void begin();
    
    /**
     * @brief Gets the current temperature.
     * Can add logic here to smooth/average readings if needed.
     */
    float getTemperature();

private:
    TempSensorDriver _driver;
};

#endif
