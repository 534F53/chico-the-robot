/*-------------------------------------------------------------*/
/*!\defgroup Movement Module
 * @{
 *
 *!\file wheelControl.c
 * \author Chen Zeng
 * \date 02/05/2017
 *
 * \mainpage Wheel Control Module
 -------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "include/motion.h"

#include <stdio.h>

// the direction of spinning thermal sensor
// ranges from 1100 ~ 4800
int sensorSpinPosition = INITIAL_PULSE_WIDTH_TICKS;
// when sensor spins to 4140, we call it an end
// sensor start spinning to the other way
int sensorReachesEnd = 0;

uint32_t *tickCountLeft;
double leftWheelTime = 0;
double leftWheelSpeed = 0;
double leftWheelDistance = 0;

uint32_t *tickCountRight;
double rightWheelTime = 0;
double rightWheelSpeed = 0;
double rightWheelDistance = 0;

// idle = 0
// forward = 1
// backward = 2
// spin left = 3
// spin right = 4
int movingDirection = 0;

// local function
/*!\brief reset the sensor
 *
 *\details position of thermal sensor is reset to center position
 */
void resetSensorPosition(void) {
	motion_servo_start(MOTION_SERVO_CENTER);
	motion_servo_set_pulse_width(MOTION_SERVO_CENTER, INITIAL_PULSE_WIDTH_TICKS);
}

// ==============================================================
/*!\brief Initialize this module
 *
 *\details Initialize the motion servo by calling the init function of motion.c
 */
void initMotion(void) {
	motion_init();
}

/*!\brief spin sensor
 *
 *\details thermal sensor spins when robot is moving
 */
void spinSensor(void) {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	motion_servo_start(MOTION_SERVO_CENTER);

	// when the robot is not moving, do not spin
	if(movingDirection == 1 || movingDirection == 2) {
		motion_servo_set_pulse_width(MOTION_SERVO_CENTER, sensorSpinPosition);

		// sensor rotate from left to right and back
		// 1100 min
		// 1140 1440 1740 2040 2340
		// 2640 Initial
		// 2940 3240 3540 3840 4140
		// 4800 max
		if (sensorReachesEnd == 0) {
			sensorSpinPosition += 300;
			if (sensorSpinPosition >= 4140) {
				sensorReachesEnd = 1;
			}
		} else {
			sensorSpinPosition -= 300;
			if (sensorSpinPosition <= 1140) {
				sensorReachesEnd = 0;
			}
		}
	}

	vTaskDelayUntil(&xLastWakeTime, (250 / portTICK_PERIOD_MS));
}

/*!\brief move forward
 *
 *\details start servomotor for left and right wheel
 */
void moveForward(void) {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	movingDirection = 1;

	motion_servo_start(MOTION_WHEEL_LEFT);
	motion_servo_start(MOTION_WHEEL_RIGHT);

	// left wheel turns counter-clockwise
	// 2.33ms / 500ns = 4660
	motion_servo_set_pulse_width(MOTION_WHEEL_LEFT, 4660);
	// right wheel turns clockwise
	// 0.55 / 500ns = 1100
	motion_servo_set_pulse_width(MOTION_WHEEL_RIGHT, MIN_PULSE_WIDTH_TICKS);

	vTaskDelayUntil(&xLastWakeTime, (250 / portTICK_PERIOD_MS));
}

/*!\brief move backwards
 *
 *\details start servomotor for left and right wheel
 */
void moveBackward(void) {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	movingDirection = 2;

	motion_servo_start(MOTION_WHEEL_LEFT);
	motion_servo_start(MOTION_WHEEL_RIGHT);

	// left wheel turns clockwise
	motion_servo_set_pulse_width(MOTION_WHEEL_LEFT, MIN_PULSE_WIDTH_TICKS);
	// right wheel turns counter-clockwise
	motion_servo_set_pulse_width(MOTION_WHEEL_RIGHT, 4660);

	vTaskDelayUntil(&xLastWakeTime, (250 / portTICK_PERIOD_MS));
}

/*!\brief move left
 *
 *\details start servomotor for left and right wheel, both left wheel & right wheel turn counter-clockwise
 */
void spinLeft(void) {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	movingDirection = 3;

	motion_servo_start(MOTION_WHEEL_LEFT);
	motion_servo_start(MOTION_WHEEL_RIGHT);

	// both left wheel & right wheel turn counter-clockwise
	motion_servo_set_pulse_width(MOTION_WHEEL_LEFT, 4660);
	motion_servo_set_pulse_width(MOTION_WHEEL_RIGHT, 4660);

	vTaskDelayUntil(&xLastWakeTime, (250 / portTICK_PERIOD_MS));
}

/*!\brief move right
 *
 *\details start servomotor for left and right wheel, both left wheel & right wheel turn clockwise
 */
void spinRight(void) {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	movingDirection = 4;

	motion_servo_start(MOTION_WHEEL_LEFT);
	motion_servo_start(MOTION_WHEEL_RIGHT);

	// both left wheel & right wheel turn clockwise
	motion_servo_set_pulse_width(MOTION_WHEEL_LEFT, MIN_PULSE_WIDTH_TICKS);
	motion_servo_set_pulse_width(MOTION_WHEEL_RIGHT, MIN_PULSE_WIDTH_TICKS);

	vTaskDelayUntil(&xLastWakeTime, (250 / portTICK_PERIOD_MS));
}

/*!\brief stop movement
 *
 *\details stops servomotor for left and right wheel, reset position of sensor
 */
void stopMotion(void) {
	// since this function will be repetitively called
	// change direction after stopping servos make sure we only stop once
	if(movingDirection != 0) {
		resetSensorPosition();
		motion_servo_stop(MOTION_SERVO_CENTER);
		motion_servo_stop(MOTION_WHEEL_RIGHT);
		motion_servo_stop(MOTION_WHEEL_LEFT);
	}

	movingDirection = 0;
}

/*!\brief update time and distance
 *
 *\details adds up the distance and updates time for each spin unit detected
 */
void updateTimeDistance(void) {
	// add distance & update time for each spin unit detected
	if (motion_enc_read(MOTION_WHEEL_LEFT, tickCountLeft) == 1) {
		leftWheelTime = (tickCountLeft[0] * 0.0000005); // 2MHz; 500ns
		leftWheelDistance += 0.54;
	}
	if (motion_enc_read(MOTION_WHEEL_RIGHT, tickCountRight) == 1) {
		rightWheelTime = (tickCountRight[0] * 0.0000005);
		rightWheelDistance += 0.54;
	}
}

/*!\brief average speed
 *
 *\details return the average speed, using total spin units detected for both left and right wheel, returned speed in meter/s
 *
 */
double getAvgSpeed(void) {
	if (leftWheelTime != 0 || rightWheelTime != 0) {
		return 0.0054 / ((leftWheelTime + rightWheelTime) / 2);
	} else {
		return 0;
	}
}


/*!\brief return distance in meters
 *
 *\details return the average moved distance of left and right wheel, unit in meters
 *
 */
double getDistance(void) {
	// sum distance divided by 2 x 100 (convert from cm to m)
	return (leftWheelDistance + rightWheelDistance) / 200;
}
/*!@}*/
