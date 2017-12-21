/*
 * led.c
 *
 */
/*-------------------------------------------------------------
 * \file led.c
 * \author Chen Zeng
 * \date 02/05/2017
 *
 * Module for handling LED lights, called by main Chico module
 -------------------------------------------------------------*/

#include "FreeRTOS.h"

/*!\brief Initialize the 3 color LED lights.
 *
 *\details Open the ports for the 3 colors LED light.
 * Note that the LED lights are active-low,
 * so they will light up when 0V is applied (after enable).
 */
void initLED(void) {
	DDRE |= _BV(DDE3); // Green
	DDRE |= _BV(DDE5); // Blue
	DDRH |= _BV(DDH3); // Red
}

/*!\brief Turn off the 3 color LED lights.
 *
 *\details Apply voltage to the 3 ports of LED lights.
 */
void clearLED(void) {
	PORTE |= _BV(PORTE3); // Green
	PORTE |= _BV(PORTE5); // Blue
	PORTH |= _BV(PORTH3); // Red
}

/*!\brief Turn on green LED light.
 *
 *\details Turn off all the lights, and then turn on green light only (apply 0V to it).
 */
void openGreenLED(void) {
	clearLED();
	PORTE &= _BV(PORTE5);
}

/*!\brief Turn on blue LED light.
 *
 *\details Turn off all the lights, and then turn on blue light only (apply 0V to it).
 */
void openBlueLED(void) {
	clearLED();
	PORTE &= _BV(PORTE3);
}

/*!\brief Turn on red LED light.
 *
 *\details Turn off all the lights, and then turn on red light only (apply 0V to it).
 */
void openRedLED(void) {
	clearLED();
	PORTH &= _BV(PORTH6);
}

/*!\brief Turn on white LED light.
 *
 *\details Turn off all the LED lights, and then turn on all 3 color lights (apply 0V to them).
 */
void openWhiteLED(void) {
	// turn off all LEDS first
	clearLED();

	// then turn on all of them
	PORTE &= _BV(PORTE5);  // green
	PORTE &= _BV(PORTE3);  // blue
	PORTH &= _BV(PORTH6);  // red
}
