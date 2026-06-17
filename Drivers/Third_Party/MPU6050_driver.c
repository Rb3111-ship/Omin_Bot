/*
 * MPU6050_driver.c
 *
 *  Created on: 5 Jun 2026
 *      Author: whp27
 */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <main.h>
#include "MPU6050_driver.h"

#define ADDRESS 0x68
#define READ_SIZE 6
#define TIMEOUT 100
#define ACC_G_VALUE 16384.0f
#define GYR_G_VALUE 131.0f
#define PI 3.14159f

extern I2C_HandleTypeDef hi2c1;

static float filtered_angle = 0.0f;
void I2C_Read_Registers(uint16_t memAddress, uint8_t *rxBuffer);

void MPU6050_INIT() {

	uint16_t devAddress = ADDRESS << 1;
	uint16_t memAddress = 0x6B;      // Register address
	uint16_t memAddSize = I2C_MEMADD_SIZE_8BIT; // 8-bit register address
	uint8_t data = 0x00;             // Data to write

	// Write 1 byte to register 0xF2
	HAL_StatusTypeDef status = HAL_I2C_Mem_Write(&hi2c1,          // I2C handle
			devAddress,      // Device address
			memAddress,      // Register address
			memAddSize,      // Register address size
			&data,           // Data to write
			1,               // Number of bytes to write
			TIMEOUT             // Timeout (ms)
			);

	if (status == HAL_OK) {
	} else {
	}
}

float getCurrentTilt() {
	uint8_t rxAccBuffer[6];
	uint16_t memAddress = 0x3B;
	I2C_Read_Registers(memAddress, rxAccBuffer);

	int16_t raw_acc_x = (int16_t) ((rxAccBuffer[0] << 8) | rxAccBuffer[1]); //acc_x (X-Axis Acceleration) forward and backward forces
//	int16_t raw_acc_y = (int16_t) ((rxAccBuffer[2] << 8) | rxAccBuffer[3]); //acc_y (Y-Axis Acceleration) left and right forces
	int16_t raw_acc_z = (int16_t) ((rxAccBuffer[4] << 8) | rxAccBuffer[5]); //acc_z (Z-Axis Acceleration) up and down forces

	//Divide the acc values by ACC_G_VALUE to get forces in Gs at the default $\pm 2g$ setting
	float raw_acc_g_x = raw_acc_x / ACC_G_VALUE;
	float raw_acc_g_z = raw_acc_z / ACC_G_VALUE;

	//calculate the angle relative to gravity, result coverted from radians to deg
	float acc_angle = atan2f(raw_acc_g_x, raw_acc_g_z) * (180.0f / PI);

	uint8_t rxGyroBuffer[6];
	memAddress = 0x43;
	I2C_Read_Registers(memAddress, rxGyroBuffer);

//	int16_t raw_gyr_x = (int16_t) ((rxGyroBuffer[0] << 8) | rxGyroBuffer[1]); //gyro_x (X-Axis Angular Velocity): Measures Roll (tilting left or right)
	int16_t raw_gyr_y = (int16_t) ((rxGyroBuffer[2] << 8) | rxGyroBuffer[3]); //gyro_y (Y-Axis Angular Velocity): Measures Pitch (falling forward or backward)
//	int16_t raw_gyr_z = (int16_t) ((rxGyroBuffer[4] << 8) | rxGyroBuffer[5]); //gyro_z (Z-Axis Angular Velocity): Measures Yaw (spinning flat on the ground).

	//Divide the acc values by GYR_G_VALUE to get forces in Gs at the default $\pm 250^\circ/\text{s}$ setting
	float gyro_y_rate = raw_gyr_y / GYR_G_VALUE;


	filtered_angle = 0.98f * (filtered_angle + (gyro_y_rate * 0.004f))
			+ 0.02f * acc_angle;

	return filtered_angle;

}

void I2C_Read_Registers(uint16_t memAddress, uint8_t *rxBuffer) {
	uint16_t devAddress = ADDRESS << 1;
	uint16_t memAddSize = I2C_MEMADD_SIZE_8BIT;

	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c1,          // I2C handle
			devAddress,      // Device address
			memAddress,      // Memory address
			memAddSize,      // Memory address size
			rxBuffer,        // Buffer to store data
			READ_SIZE,               // Number of bytes to read
			TIMEOUT             // Timeout (ms)
			);

	if (status == HAL_OK) {
		// Success! Data is in rxBuffer

	} else {
		// Error handling

	}

}
