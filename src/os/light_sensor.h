#ifndef _LIGHT_SENSOR_H_
#define _LIGHT_SENSOR_H_

typedef enum { Sensor0 = 0, Sensor1 = 1 } ActiveSensor;

// Get sensor receiving the most light.
ActiveSensor read_max_light_sensor(void);

#endif