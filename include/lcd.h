/*
 * lcd.h
 *
 *  Created on: Jan 27, 2017
 *      Author: asus1d
 */

#ifndef INCLUDE_LCD_H_
#define INCLUDE_LCD_H_

void initLCD(void);
void clearLCD(void);
void commandLCD(int command);
void printLCD(char *firstLine, char *secondLine);

#endif /* INCLUDE_LCD_H_ */
