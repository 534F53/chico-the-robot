/*-----------------------------------------------------------------
 * \file lcd.c
 * \author Chen Zeng
 * \date 02/05/2017
 *
 * Module for handling lcd display, called by main Chico module
 ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "usartSerial.h"

/* HD44780 commands we used in this project */
#define COMMAND_SPECIAL     0x7C
#define COMMAND_CHAR        0xFE
#define DISPLAY_CLEAR       0x01
#define DISPLAY_OFF         0x08
#define DISPLAY_ON          0x0C
#define CURSOR_LINE1        0x80
#define CURSOR_LINE2        0xC0

/*-----------------------------------------------------------*/
void clearLCD(void);
void commandLCD(int command);

/*!\brief Initialize the LCD.
 *
 *\details
 * - turn on usart1 serial port for communication
 * - enable ports
 * - turn on and clear LCD display
 */
void initLCD(void) {
	DDRD |= _BV(DDD3);
	PORTD |= _BV(PORTD3);

	commandLCD(DISPLAY_ON);
	clearLCD();
}

/*\brief Send the clear command to LCD.
 */
void clearLCD(void) {
	usartWrite(USART1_ID, COMMAND_CHAR);
	usartWrite(USART1_ID, DISPLAY_CLEAR);
}

/*\brief Send a specific command to LCD.
 *
 *\details
 * @param command A 8-bit instruction to send to LCD.
 */
void commandLCD(int command) {
	usartWrite(USART1_ID, COMMAND_CHAR);
	usartWrite(USART1_ID, command);
}

/*!\brief Print a 2-line message on the LCD.
 *
 *\details
 * - clear previous displayed message on LCD
 * - print 2 strings on 2 lines separately
 *
 * @param *firstLine A string to be printed in the 1st line of LCD.
 * @param *secondLine A string to be printed in the 2st line of LCD.
 */
void printLCD(char *firstLine, char *secondLine) {
	// clear previous displayed message
	clearLCD();
	// move cursor to the beginning of 1st line
	commandLCD(CURSOR_LINE1);
	usart_fprint(USART1_ID, (uint8_t *) firstLine);
	// move cursor to the beginning of 2nd line
	commandLCD(CURSOR_LINE2);
	usart_fprint(USART1_ID, (uint8_t *) secondLine);
}
