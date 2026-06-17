/*
 * ultrasonic.c
 *
 *  Created on: 5 Jun 2026
 *      Author: whp27
 */

#include "ultrasonic.h"
#include "stm32f4xx_hal.h"
#include "main.h"

extern TIM_HandleTypeDef htim1;
uint32_t startTick = 0;

typedef enum {
	RISING, FALLING
} Echostate_t;

static Echostate_t echo_state = RISING;

volatile UltrasonicSensor_t g_frontSensor;
volatile UltrasonicSensor_t g_rearSensor;

void init_timer() {
	HAL_TIM_Base_Start(&htim1);
	HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_3);

	HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_4);
}

void reset_EchoState(){
	echo_state = RISING;
}

void Ultrasonic_TriggerFront(void) {
	startTick = __HAL_TIM_GET_COUNTER(&htim1);
	TriggerHigh(US_FRONT_TRIG_GPIO_Port, US_FRONT_TRIG);

	while ((__HAL_TIM_GET_COUNTER(&htim1) - startTick) <= 10)
		;

	TriggerLow(US_FRONT_TRIG_GPIO_Port, US_FRONT_TRIG);

}

void Ultrasonic_TriggerRear(void) {
	startTick = __HAL_TIM_GET_COUNTER(&htim1);

	TriggerHigh(US_REAR_TRIG_GPIO_Port, US_REAR_TRIG);

	while ((__HAL_TIM_GET_COUNTER(&htim1) - startTick) <= 10)
		;
	TriggerLow(US_REAR_TRIG_GPIO_Port, US_REAR_TRIG);

}

void TriggerHigh(GPIO_TypeDef *port, uint16_t pin) {
	HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
}

void TriggerLow(GPIO_TypeDef *port, uint16_t pin) {
	HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}

uint32_t CalculatePulseWidth(uint32_t start, uint32_t finish) {
	uint32_t timer_max = __HAL_TIM_GET_AUTORELOAD(&htim1);

	if (finish >= start) {
		return finish - start;
	}

	return (timer_max - start) + finish + 1;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM1) {
		// ultrasonic processing
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3) { // Front sensor
			if (echo_state == RISING) {
				g_frontSensor.start_time = HAL_TIM_ReadCapturedValue(htim,
				TIM_CHANNEL_3);
				echo_state = FALLING;
			} else if (echo_state == FALLING) {
				g_frontSensor.finish_time = HAL_TIM_ReadCapturedValue(htim,
				TIM_CHANNEL_3);

				g_frontSensor.pulse_width = CalculatePulseWidth(g_frontSensor.start_time,
						g_frontSensor.finish_time);

				g_frontSensor.calculation_complete = 1;

				echo_state = RISING;
			}

		}
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4) {
			// Rear sensor
			if (echo_state == RISING) {
				g_rearSensor.calculation_complete = 0;
				g_rearSensor.start_time = HAL_TIM_ReadCapturedValue(htim,
				TIM_CHANNEL_4);
				echo_state = FALLING;
			} else if (echo_state == FALLING) {
				g_rearSensor.finish_time = HAL_TIM_ReadCapturedValue(htim,
				TIM_CHANNEL_4);

				g_rearSensor.pulse_width = CalculatePulseWidth(g_rearSensor.start_time,
						g_rearSensor.finish_time);

				g_rearSensor.calculation_complete = 1;

				echo_state = RISING;
			}

		}
	}
}

