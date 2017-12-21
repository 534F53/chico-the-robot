/*
 * thermalSensor.h
 *
 *  Created on: Jan 27, 2017
 *      Author: asus1d
 */

#ifndef INCLUDE_THERMALSENSOR_H_
#define INCLUDE_THERMALSENSOR_H_

void initThermal(void);
void readTemperatures(void);
int getTemperatureAvg(void);
int getSensorValue(int sensor);
int getLeftAvg(void);
int getRightAvg(void);
int getCenterAvg(void);
int closeToHeat(void);

#endif /* INCLUDE_THERMALSENSOR_H_ */
