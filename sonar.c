/*
 * sonar.c
 *
 *  Created on: Mar 22, 2017
 *      Author: Chen Zeng
 *
 *      !\defgroup Sonar Module
 *      !\file wheelControl.c
 *      \date 04/09/2017
 *
 *       \mainpage Sonar Module
 */


#include <util/delay.h>
#include "FreeRTOS.h"
#include "include/custom_timer.h"


/*!\brief Initialize the sonar module
 *
 *\details Module timer is initialized for sonar.
 */
void initSonar(void) {
	initialize_module_timer0();
}


/*!\brief Return the distance of target
 *
 *\details Returns the distance of target using sonar, where the distance is the distance of the echo * the speed in air and divided by 2
 */
double getSonarDistance(void) {

	// make port output
	DDRA |= 0b00000001;

	// Give a short LOW pulse beforehand to ensure a clean HIGH pulse
	// custom_timer does not support microsecond delays
	// so use delay from AVR util
	PORTA &= 0b11111110;
	_delay_us(2);

	// typical T-out from PING))) documentation = 5us
	PORTA |= 0b00000001;
	_delay_us(5);
	PORTA &= 0b11111110;

	// make port input
	DDRA &= 0b11111110;

	// wait for signal go high, start time count
	loop_until_bit_is_set(PINA, PINA0);
	long echoStart = time_in_microseconds();
	// wait for signal go back to low, stop time count
	loop_until_bit_is_clear(PINA, PINA0);
	long echoStop = time_in_microseconds();

	long tIn = echoStop - echoStart;

	// sound speed in air = 340m/s = 0.034cm/us
	// round-trip so divide by 2 to get distance
	double distance = tIn * 0.034 / 2;

	return distance;
}
