/*
 * thermalSensor.c
 *
 */

/*-----------------------------------------------------------------
 * \file thermalSensor.c
 * \author Chen Zeng
 * \date 02/05/2017
 *
 * Module for handling thermal sensor, called by main Chico module
 ------------------------------------------------------------------*/

#include "i2cMultiMaster.h"

/* I2C addresses */
// 0xC0 as master and 0x01 as bearing
// taken from the I2C example of freeRTOS Module Documentation
#define I2C_MASTER      0xBA
#define I2C_THERMAL_W   0xD0
#define I2C_THERMAL_R   0xD1
#define I2C_BEARING     0x00

/* Sensors number */
// just integers 1-9, not addresses
#define SENSOR_AMBIENT	0x01
#define SENSOR_PIXEL1	0x02
#define SENSOR_PIXEL2	0x03
#define SENSOR_PIXEL3	0x04
#define SENSOR_PIXEL4	0x05
#define SENSOR_PIXEL5	0x06
#define SENSOR_PIXEL6	0x07
#define SENSOR_PIXEL7	0x08
#define SENSOR_PIXEL8	0x09

// Thermal array initializer
uint8_t thermalSensors[9][2] = {
	{I2C_THERMAL_W, SENSOR_AMBIENT},
	{I2C_THERMAL_W, SENSOR_PIXEL1},
	{I2C_THERMAL_W, SENSOR_PIXEL2},
	{I2C_THERMAL_W, SENSOR_PIXEL3},
	{I2C_THERMAL_W, SENSOR_PIXEL4},
	{I2C_THERMAL_W, SENSOR_PIXEL5},
	{I2C_THERMAL_W, SENSOR_PIXEL6},
	{I2C_THERMAL_W, SENSOR_PIXEL7},
	{I2C_THERMAL_W, SENSOR_PIXEL8}
};

// Result array for read temperatures
uint8_t thermalValues[9][2];

// I2C read initializer
uint8_t thermalRead[2] = {I2C_THERMAL_R, I2C_BEARING};

int temperatureSum = 0;
int temperatureAvg = 0;

/*!\brief Initialize the thermal sensors.
 *
 *\details Set up I2C master to enable communication with thermal sensors.
 */
void initThermal(void) {
	I2C_Master_Initialise(I2C_MASTER);
}

/*!\brief Read current temperatures from thermal sensors.
 *
 *\details
 * - Go through each thermal temperature, request and read data from them through I2C,
 *   store the data in the result array.
 * - Calculate average temperature after data collecting.
 */
void readTemperatures(void) {
	temperatureSum = 0;

	for (int i = 0; i < 9; i++) {
		I2C_Master_Start_Transceiver_With_Data((thermalSensors[i]), 2);
		I2C_Master_Start_Transceiver_With_Data(thermalRead, 2);
		I2C_Master_Get_Data_From_Transceiver(thermalValues[i], 2);
		temperatureSum += thermalValues[i][1];
	}

	temperatureAvg = (int) temperatureSum / 9;
}

/*!\brief Get calculated average temperature.
 *
 *\details
 * return int The average temperature.
 */
int getTemperatureAvg(void) {
	return temperatureAvg;
}

/*!\brief Get read value from a specific sensor
 *
 *\details
 * @param sensor The sensor number to read temperature.
 * return int Temperature value read from the specific sensor.
 */
int getSensorValue(int sensor) {
	return thermalValues[sensor][1];
}

/*!\brief Get average value of left 4 thermal sensors
 *
 *\details
 * return int Average temperature read from left 4 thermal sensors.
 */
int getLeftAvg(void) {
	int leftSum = 0;
	for(int i = 5; i < 9; i++) {
		leftSum += thermalValues[i][1];
	}
	return leftSum / 4;
}

/*!\brief Get average value of right 4 thermal sensors
 *
 *\details
 * return int Average temperature read from right 4 thermal sensors.
 */
int getRightAvg(void) {
	int rightSum = 0;
	for(int i = 1; i < 5; i++) {
		rightSum += thermalValues[i][1];
	}
	return rightSum / 4;
}

/*!\brief Get average value of center 2 thermal sensors
 *
 *\details
 * return int Average temperature read from center 2 thermal sensors.
 */
int getCenterAvg(void) {
	return (thermalValues[4][1] + thermalValues[5][1]) / 2;
}

/*!\brief detect if average value of pixel sensors is higher than ambient temperature
 * if true, than heat source is ahead
 *
 *\details
 * return int return 0 if false, return 1 if true
 */
int closeToHeat(void) {
	if (temperatureAvg > thermalValues[0][1] + 1) {
		return 1;
	}
	else {
		return 0;
	}
}
