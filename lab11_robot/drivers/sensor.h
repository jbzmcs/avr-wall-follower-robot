// drivers/sensor.h - HC-SR04 ultrasonic driver (distances in inches)
#ifndef SENSOR_H
#define SENSOR_H

void sensor_init(void);

// Single measurement. Returns distance in inches,
// or 0.0 on timeout / out-of-range reading.
double sensor_read_inches(void);

#endif
