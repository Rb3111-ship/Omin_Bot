/*
 * safetyWatchdogTask.c
 *
 *  Created on: 5 Jun 2026
 *      Author: whp27
 */
#include "ultrasonic.h"
#include "FreeRTOS.h"
#include "task.h"
#include "freeRTOS_tasks.h"


#define US_CONST 58

typedef enum {
	US_FRONT_IDLE,

	US_WAIT_FRONT,

	US_REAR_IDLE,

	US_WAIT_REAR

} UltrasonicState_t;

static UltrasonicState_t current_state = US_FRONT_IDLE;
static uint32_t front_start_wait = 0;
static uint32_t rear_start_wait = 0;

void safetyWatchdogTask(){
	TickType_t xLastWakeTime = xTaskGetTickCount();
	const TickType_t xPeriod = pdMS_TO_TICKS(10);
	init_timer();
	for (;;) {

		if (current_state == US_FRONT_IDLE) {
			Ultrasonic_TriggerFront();
			front_start_wait = xTaskGetTickCount();
			current_state = US_WAIT_FRONT;
		} else if (current_state == US_WAIT_FRONT) {
			if (g_frontSensor.calculation_complete == 1) {
				g_frontSensor.calculation_complete = 0;
				uint16_t uSeconds = g_frontSensor.pulse_width;
				uint16_t distance = calculate_distance(uSeconds); //TODO: USE DISTANCE TO CONTROL BOT
				current_state = US_REAR_IDLE;
			} else if ((xTaskGetTickCount() - front_start_wait)
					> pdMS_TO_TICKS(50)) {
				reset_EchoState();
				current_state = US_REAR_IDLE;

				//TODO: TRIGGER A FAUT SYSTEM ON THE CAN
			}

		} else if (current_state == US_REAR_IDLE) {
			Ultrasonic_TriggerRear();
			rear_start_wait = xTaskGetTickCount();
			current_state = US_WAIT_REAR;
		}

		else if (current_state == US_WAIT_REAR) {
			if (g_rearSensor.calculation_complete == 1) {
				g_rearSensor.calculation_complete = 0;
				uint16_t uSeconds = g_rearSensor.pulse_width;
				uint16_t distance = calculate_distance(uSeconds); //TODO: USE DISTANCE TO CONTROL BOT
				current_state = US_FRONT_IDLE;
			} else if ((xTaskGetTickCount() - rear_start_wait)
					> pdMS_TO_TICKS(50)) {
				reset_EchoState();
				current_state = US_FRONT_IDLE;

				//TODO: TRIGGER A FAUT SYSTEM ON THE CAN

			}
		}
		vTaskDelayUntil(&xLastWakeTime, xPeriod);

	}
}

uint16_t calculate_distance(uint16_t uSeconds) {

	return uSeconds / US_CONST;
}

