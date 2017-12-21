/*
 * wheelControl.h
 *
 *  Created on: Mar 6, 2017
 *      Author: asus1d
 */

#ifndef INCLUDE_WHEELCONTROL_H_
#define INCLUDE_WHEELCONTROL_H_

void initMotion();
void spinSensor();
void moveForward();
void moveBackward();
void spinLeft();
void spinRight();
void stopMotion();
void updateTimeDistance();
double getAvgSpeed();
double getDistance();

#endif /* INCLUDE_WHEELCONTROL_H_ */
