/*
 * \defgroup Main Chico Module
 * @{
 *
 * \mainpage Main Chico Module
 */

/*----------------------------------------------------------------
 * \file main.c
 * \author Chen Zeng
 * \date 02/05/2017
 *
 * Main module for Chico, handles the task scheduler, led display,
 * lcd lights, and thermal sensor
 -----------------------------------------------------------------*/

/* --Includes-- */
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

/*AVR includes*/
#include <avr/io.h>

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "util/delay.h"

/* serial interface include file. */
#include "usartserial.h"
#include "include/wireless_interface.h"

/* hardware module include files. */
#include "include/lcd.h"
#include "include/led.h"
#include "include/thermalSensor.h"
#include "include/wheelControl.h"
#include "include/sonar.h"

USART_ID usart_zero = USART0_ID;							/*!<USART for serial terminal communication.*/
USART_ID usart_one = USART1_ID;								/*!<USART for LCD communication.*/
USART_ID usart_two = USART2_ID;								/*!<USART for Gainspan WiFi communication.*/

/*-----------------------------------------------------------*/

/* --Prototypes-- */
int main(void) __attribute__((OS_main));
void taskHandleHttp(void *pvParameters);
void taskSpeedMonitor(void *pvParameters);
void taskBehavior(void *pvParameters);
void taskLCD(void *pvParameters);
void vApplicationStackOverflowHook( TaskHandle_t xTask, portCHAR *pcTaskName);

int usartfd;

double dis = 0;
int closeHeat = 0;
int moveCount = 0;
int command = 0;

int state = 0;
// state: 0 searching, 1 attached, 2 panic


/*!\brief Main function (program entry).
 *
 *\details Initializes serial port and hardware components, and enables interrupts in order to set up the task scheduler
 *\details Priority is given to the LCD before the thermal sensor.
 *
 *
 */
int main(void)
{
	/*Initialize USART0 - serial terminal.*/
	usart_zero = usartOpen(USART0_ID, BAUD_RATE_115200, portSERIAL_BUFFER_TX, portSERIAL_BUFFER_RX);

	/*Initialize USART1 for LCD/HD44780U controller communication.*/
	usart_one = usartOpen(USART1_ID, BAUD_RATE_9600, portSERIAL_BUFFER_TX, portSERIAL_BUFFER_RX);

	/*Initialize USART2 for Gainspan based WiFi Shield.*/
	usart_two = usartOpen(USART2_ID, BAUD_RATE_9600, portSERIAL_BUFFER_TX, portSERIAL_BUFFER_RX);

	// enable interrupts
	taskENABLE_INTERRUPTS();
	portENABLE_INTERRUPTS();

	gs_initialize_module(usart_two, BAUD_RATE_9600, usart_zero, BAUD_RATE_115200);
	gs_set_wireless_ssid("HP-Print-900-LaserCat");
	gs_activate_wireless_connection();

	configure_web_page("Chico", "Chico Control", HTML_DROPDOWN_LIST);

	add_element_choice('A', "Attach"); // Attach
	add_element_choice('S', "Stop"); // Stop
	add_element_choice('F', "Forward"); // Up arrow, forward
	add_element_choice('B', "Backward"); // Down, backward
	add_element_choice('L', "Left"); // Counter clockwise, spin left
	add_element_choice('R', "Right"); // Clockwise, spin right

	start_web_server();
	_delay_ms(3000);

	// enable hardware components
	initLCD();
	initLED();
	initMotion();
	initSonar();

	xTaskCreate(
		taskHandleHttp,
		(const portCHAR *)"SRPRWFRQ",
		1024,
		NULL,
		3,
		NULL);

	xTaskCreate(
		taskSpeedMonitor,
		(const portCHAR *)"SpeedMonitor",
		128,
		NULL,
		3,
		NULL);

	xTaskCreate(
		taskBehavior,
		(const portCHAR *)"Behavior",
		256,
		NULL,
		3,
		NULL);

	xTaskCreate(
		taskLCD,
		(const portCHAR *)"LCD",
		160,
		NULL,
		3,
		NULL);

	// start scheduled tasks
	vTaskStartScheduler();
}


/*! \brief Serve client response/request
 *
 * \details Serve client request, submission of user selection from web-page.
 * Client response/request are stored in a ring buffer, which are read and processed.
 *
 *
 * @return void
 *
 */
void serve_client_request(void)
{
	char client_request = '\0';
	uint8_t buffer_read_counter = 0;
	/*Read client request*/
	/*Its a ring buffer so read all the positions to ensure all requests are handled*/
	/*serve each request, if there*/
	for (buffer_read_counter = 0; buffer_read_counter  < RING_BUFFER_SIZE; buffer_read_counter ++){
		client_request = get_next_client_response();
		if (client_request == 'S'){
			command = 0;
		}
		else if (client_request == 'A'){
			command = 1;
		}
		else if (client_request == 'F'){
			command = 2;
		}
		else if (client_request == 'B'){
			command = 3;
		}
		else if (client_request == 'L'){
			command = 4;
		}
		else if (client_request == 'R'){
			command = 5;
		}
	}
}


/*! \brief Task - Accept and process HTTP requests of wireless connection.
 *
 * \details Task - Accept, process HTTP requests
 * request and process client response to the web-page via submission i.e. user selection.
 *
 *
 * @return void
 *
 *
 */
void taskHandleHttp(void *pvParameters) {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	while(1) {
		/*Accept and serve the HTTP request by sending web page*/
		process_client_request();
		/*Serve client response/request:submission of user selection from web-page */
		serve_client_request();
		/*Relinquish the processor*/

		vTaskDelayUntil(&xLastWakeTime, (5500 / portTICK_PERIOD_MS)); //Cycle 5500ms
	}
}


/* ---------------------------------------------------------------------------*/
/*!\brief taskSpeedMonitor
 *
 * \details function to called in wheelControl module to keep updating the speed and distance
 *
 *   @param *pvParameters
 *
 *----------------------------------------------------------------------------*/
void taskSpeedMonitor(void *pvParameters) {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	while(1) {
		// call function in wheelControl module to keep updating the travel speed and distance
		updateTimeDistance();
		vTaskDelayUntil(&xLastWakeTime, (100 / portTICK_PERIOD_MS)); //Cycle 100ms
	}
}


/* ---------------------------------------------------------------------------*/
/*!\brief taskBehavior
 *
 * \details initializes thermal sensor scanning
 * control the behavior of chico
 *
 *   @param *pvParameters
 *
 *----------------------------------------------------------------------------*/
void taskBehavior(void *pvParameters) {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	initThermal();

	while(1) {
		// start thermal sensor scanning
		spinSensor();
		readTemperatures();
		updateTimeDistance();
		dis = getSonarDistance();
		closeHeat = closeToHeat();

		if (command == 0) { // Stop
			stopMotion();
			openWhiteLED();
		}
		else if (command == 1) { // Attachment mode
			// searching state, spin left and right to search heat source
			// if found heat source, enter attached state
			// if cannot find heat source in a while, enter panic state
			if (state == 0) {
				openBlueLED();
				if (moveCount > 24) {
					moveCount = 0;
					state = 2;
				}
				else {
					if (moveCount % 12 < 6) {
						spinLeft();
					}
					else {
						spinRight();
					}
					if (closeHeat == 1 && dis < 40) {
						moveCount = 0;
						state = 1;
					}
				}
				moveCount++;
			}
			// attached state
			// motion stopped
			// if heat source moves away from heat sensor,
			// chico will try to catch up by moving forward
			// chico will not exactly follow the movement of heat source since this part is optional in the requirements
			// but this behavior is good for following a heat source that moves straight
			//
			// if chico fails to catch up, it will go back to searching state
			else if (state == 1) {
				openGreenLED();
				stopMotion();
				if (closeHeat == 1 && dis < 40) {
					moveCount = 0;
					stopMotion();
				}
				// turn direction if
				else if (dis < 40) {
					spinLeft();
				}
				else {
					moveCount++;
				}

				if (moveCount > 6) {
					moveForward();

					if (moveCount > 20) {
						moveCount = 0;
						state = 0;
					}
				}
			}
			// panic state, keep spinning right for some time and go back to searching state
			else if (state == 2) {
				openRedLED();
				spinRight();
				moveCount++;
				if (moveCount > 20) {
					moveCount = 0;
					state = 0;
				}
			}
		}
		else if (command == 2) { // Forward
			moveForward();
			openGreenLED();
			moveCount++;
			if (moveCount > 5) {
				moveCount = 0;
				command = 0;
			}
		}
		else if (command == 3) { // Backward
			moveBackward();
			openRedLED();
			moveCount++;
			if (moveCount > 5) {
				moveCount = 0;
				command = 0;
			}
		}
		else if (command == 4) { // Left
			spinLeft();
			openBlueLED();
			moveCount++;
			if (moveCount >= 1) {
				moveCount = 0;
				command = 0;
			}
		}
		else if (command == 5) { // Right
			spinRight();
			openBlueLED();
			moveCount++;
			if (moveCount >= 1) {
				moveCount = 0;
				command = 0;
			}
		}

		vTaskDelayUntil(&xLastWakeTime, (250 / portTICK_PERIOD_MS));  //Cycle 250ms
	}

}

/*\brief LCD task.
 *
 *\details Update LCD according to the current temperatures recorded using two buffers.
 *
 * @param *pvParameters A value that will passed into the created task as the task's parameter.
 */
void taskLCD(void *pvParameters) {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	// LCD message buffers
	char line1Buffer[16];
	char line2Buffer[16];

	while (1) {
		// write messages to buffers
		// first line: S - average speed, D - traveled distance
		// second line: A - ambient temperature, L - left average temperature, R - right average temperature
		sprintf(line1Buffer, "S:%.2f, D:%.2f",
			getAvgSpeed(),
			getDistance());
			//getDistance());
		sprintf(line2Buffer, "A:%d, L:%d, R:%d",
			getSensorValue(0),
			getLeftAvg(),
			getRightAvg());
		// update LCD display
		printLCD(line1Buffer, line2Buffer);

		// task interval
		vTaskDelayUntil(&xLastWakeTime, (500 / portTICK_PERIOD_MS));  //Cycle 500ms
	}
}

/*\brief Application Stack Overflow
 *
 *\details
 *
 * @param xTask A task from the task handler
 * @param *pcTaskName Identification of port
 */
void vApplicationStackOverflowHook (TaskHandle_t xTask, portCHAR *pcTaskName) {
	DDRE &= ~_BV(DDE3); 		/* 	Turn Green Off	*/
	DDRE &= ~_BV(DDE5); 		/* 	Turn Blue Off	*/
	DDRH &= ~_BV(DDH3); 		/*	Turn Red Off	*/
}
/*!@}*/
