/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32f4xx_it.c
 * @brief   Interrupt Service Routines.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "freeRTOS_tasks.h"
#include "MCP2515_driver.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim5;

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void) {
	/* USER CODE BEGIN NonMaskableInt_IRQn 0 */

	/* USER CODE END NonMaskableInt_IRQn 0 */
	/* USER CODE BEGIN NonMaskableInt_IRQn 1 */
	while (1) {
	}
	/* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
 * @brief This function handles Hard fault interrupt.
 */
void HardFault_Handler(void) {
	/* USER CODE BEGIN HardFault_IRQn 0 */

	/* USER CODE END HardFault_IRQn 0 */
	while (1) {
		/* USER CODE BEGIN W1_HardFault_IRQn 0 */
		/* USER CODE END W1_HardFault_IRQn 0 */
	}
}

/**
 * @brief This function handles Memory management fault.
 */
void MemManage_Handler(void) {
	/* USER CODE BEGIN MemoryManagement_IRQn 0 */

	/* USER CODE END MemoryManagement_IRQn 0 */
	while (1) {
		/* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
		/* USER CODE END W1_MemoryManagement_IRQn 0 */
	}
}

/**
 * @brief This function handles Pre-fetch fault, memory access fault.
 */
void BusFault_Handler(void) {
	/* USER CODE BEGIN BusFault_IRQn 0 */

	/* USER CODE END BusFault_IRQn 0 */
	while (1) {
		/* USER CODE BEGIN W1_BusFault_IRQn 0 */
		/* USER CODE END W1_BusFault_IRQn 0 */
	}
}

/**
 * @brief This function handles Undefined instruction or illegal state.
 */
void UsageFault_Handler(void) {
	/* USER CODE BEGIN UsageFault_IRQn 0 */

	/* USER CODE END UsageFault_IRQn 0 */
	while (1) {
		/* USER CODE BEGIN W1_UsageFault_IRQn 0 */
		/* USER CODE END W1_UsageFault_IRQn 0 */
	}
}

/**
 * @brief This function handles Debug monitor.
 */
void DebugMon_Handler(void) {
	/* USER CODE BEGIN DebugMonitor_IRQn 0 */

	/* USER CODE END DebugMonitor_IRQn 0 */
	/* USER CODE BEGIN DebugMonitor_IRQn 1 */

	/* USER CODE END DebugMonitor_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
 * @brief This function handles TIM1 capture compare interrupt.
 */
void TIM1_CC_IRQHandler(void) {
	/* USER CODE BEGIN TIM1_CC_IRQn 0 */

	/* USER CODE END TIM1_CC_IRQn 0 */
	HAL_TIM_IRQHandler(&htim1);
	/* USER CODE BEGIN TIM1_CC_IRQn 1 */

	/* USER CODE END TIM1_CC_IRQn 1 */
}

/**
 * @brief This function handles TIM5 global interrupt.
 */
void TIM5_IRQHandler(void) {
	/* USER CODE BEGIN TIM5_IRQn 0 */

	/* USER CODE END TIM5_IRQn 0 */
	HAL_TIM_IRQHandler(&htim5);
	/* USER CODE BEGIN TIM5_IRQn 1 */

	/* USER CODE END TIM5_IRQn 1 */
}

/**
 * @brief This function handles EXTI line 0 interrupt.
 */
void EXTI0_IRQHandler(void) {
	/* USER CODE BEGIN EXTI0_IRQn 0 */
	/* USER CODE END EXTI0_IRQn 0 */

	// This official ST HAL function automatically clears the pending hardware register bit
	// and immediately jumps directly down to execute our custom HAL_GPIO_EXTI_Callback!
	HAL_GPIO_EXTI_IRQHandler(CAN_INT_Pin);

	/* USER CODE BEGIN EXTI0_IRQn 1 */
	/* USER CODE END EXTI0_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/**
 * @brief  EXTI line detection callback from the MCP2515 INT Pin.
 * @param  GPIO_Pin: Specifies the pin connected to MCP2515 interrupt
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	// 1. Verify this interrupt came from our physical CAN controller pin
	if (GPIO_Pin == CAN_INT_Pin) {
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		// Instantiate the library's required frame structure
		can_frame rx_frame;

		// 2. Poll the MCP2515 internal interrupt flags via SPI
		// The driver checks the CANINTF register under the hood
		uint8_t interrupt_status = MCP_getInterrupts();

		// 3. Check if the interrupt status bit indicates a message is pending in RXB0 or RXB1
		// Usually, Bit 0 (0x01) is RX0IF and Bit 1 (0x02) is RX1IF.
		// We will process if *any* receive flag is raised (0x03 mask)
		if (interrupt_status & 0x03) {
			// Read message automatically clears the controller's specific RX buffer flag
			if (MCP_readMessage(&rx_frame) == ERROR_OK) {
				// --- INTERRUPT PARSING ENGINE ---

				// Case A: Navigation Command Frame received from Pi
				if (rx_frame.can_id == 0x200) {
					NavCommand_t nav_cmd;

					// Decode bytes back to float values using type-safe memory copying
					memcpy(&nav_cmd.target_velocity, &rx_frame.data[0], 4);
					memcpy(&nav_cmd.target_yaw_rate, &rx_frame.data[4], 4);

					// Push to FreeRTOS Queue, unblocking ControlTask instantly if it's waiting
					xQueueSendToBackFromISR(NavigationQueue, &nav_cmd,
							&xHigherPriorityTaskWoken);
				}

				// Case B: 20ms Heartbeat Keep-Alive Frame from Pi
				else if (rx_frame.can_id == 0x700) {
					// Confirm our explicit 1-byte payload rule (0xAA)
					if (rx_frame.data[0] == 0xAA) {
						// Direct Task Notification using the designated bit mask
						xTaskNotifyFromISR((TaskHandle_t) safetyTaskHandle,
						NOTIFY_BIT_HEARTBEAT, eSetBits,
								&xHigherPriorityTaskWoken);
					}
				}
			}
		}

		// 4. Clear all remaining global interrupt flags on the MCP2515 hardware
		// This drops the physical INT pin back to logic HIGH so it can trip again later.
		MCP_clearInterrupts();

		// 5. Context Switch Enforcement
		// If xHigherPriorityTaskWoken was set to pdTRUE by an RTOS API,
		// this forces our Cortex-M4 to bypass the current task and jump straight
		// to our high-priority loops on exit.
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}
/* USER CODE END 1 */
