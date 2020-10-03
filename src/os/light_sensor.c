#include "light_sensor.h"

#include <avr/io.h>

#define READ_MASK (1 << ACO)

ActiveSensor read_max_light_sensor(void)
{
    return ACSR & READ_MASK ? Sensor1 : Sensor2;
}