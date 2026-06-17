/*
 * ControlTask.c
 *
 *  Created on: 5 Jun 2026
 *      Author: whp27
 */

#include "MPU6050_driver.h"
#include "freeRTOS_tasks.h"
#include "FreeRTOS.h"
#include "task.h"

controlTask() {


	// 1. One-time initialisations (Sensors, PID, Motors)
	MPU6050_INIT();

	// 2. Setup the deterministic 250Hz absolute delay
	uint32_t tick_wake_time = osKernelGetTickCount();
	const uint32_t period_ms = 4;

	// 3. The Real-Time Loop
	for (;;) {
		// Read IMU & Encoders
		// Calculate PID
		// Update Motor PWM

		// Wait exactly 4ms from the last wake time
	    osDelayUntil(&tick_wake_time, period_ms);

	}
}
