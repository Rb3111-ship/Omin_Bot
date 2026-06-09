/*
 * pid_controller.c
 *
 *  Created on: 5 Jun 2026
 *      Author: whp27
 */

#include <string.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include "MPU6050_driver.h"
#include "pid_controller.h"

#define PWM_CAP 999.0f

float active_Kp = 30.0f;
float active_Ki = 2.5f;
float active_Kd = 2.5f;
static float timestep = 0.004;
static float prev_error = 0;
static float balance_Output = 0;
static float target_Angle = 0.0f; //change this value to move forward and backward
static float integrator = 0;

void pid(uint8_t dir_angle, uint16_t steering_Offset) {

	target_Angle = dir_angle;

	float error = target_Angle - getCurrentTilt(); //  getCurrentTilt() from mpu driver

	float p_out = active_Kp * error;

	integrator += (error * timestep);

	float i_out = active_Ki * integrator;

	float slope = (error - prev_error) / timestep;

	float d_out = active_Kd * slope;

	balance_Output = p_out + i_out + d_out;

	prev_error = error;

	if (balance_Output < 0) { // negative total_output moves bot backwards
		MOTOR_L_IN1 = 1;
		MOTOR_L_IN2 = 0;
		MOTOR_R_IN1 = 1;
		MOTOR_R_IN2 = 0;

	} else {
		MOTOR_L_IN1 = 0;
		MOTOR_L_IN2 = 1;
		MOTOR_R_IN1 = 0;
		MOTOR_R_IN2 = 1;
	}

	float motor_output1 = fabsf(balance_Output) + steering_Offset;
	float motor_output2 = fabsf(balance_Output) - steering_Offset;

	MOTOR_PWM_L->CCR1 = motor_output1 > PWM_CAP ? PWM_CAP : motor_output1;
	MOTOR_PWM_R->CCR1 = motor_output2 > PWM_CAP ? PWM_CAP : motor_output2;
}

