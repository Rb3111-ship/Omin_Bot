/*
 * freeRTOS_tasks.h
 *
 *  Created on: 16 Jun 2026
 *      Author: whp27
 */

#ifndef INC_FREERTOS_TASKS_H_
#define INC_FREERTOS_TASKS_H_


#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

/* ==========================================
   1. HIERARCHICAL SYSTEM FAULT BITMASKS (Focus B)
   ========================================== */
#define FAULT_NONE          ((uint8_t)0x00)
#define FAULT_IMU_TIMEOUT   ((uint8_t)0x01) // Bit 0
#define FAULT_US_LEFT_TO    ((uint8_t)0x02) // Bit 1
#define FAULT_US_RIGHT_TO   ((uint8_t)0x04) // Bit 2
#define FAULT_MOTOR_DRV     ((uint8_t)0x08) // Bit 3
#define FAULT_EXT_CORRUPT   ((uint8_t)0x10) // Bit 4
#define FAULT_LIPO_LOW      ((uint8_t)0x20) // Bit 5


#define NOTIFY_BIT_HEARTBEAT    (1 << 0)

/* ==========================================
   2. SYSTEM OPERATIONAL STATE MACHINE (Focus A)
   ========================================== */
typedef enum {
    STATE_INITIALIZING = 0,
    STATE_BALANCING,
    STATE_FAILSAFE_STAGE1_BRAKE,   // 0-200ms Active Braking
    STATE_FAILSAFE_STAGE2_DESCENT, // 200-500ms Controlled Tip down
    STATE_FAILSAFE_STAGE3_ISOLATE  // 500ms+ Drive STBY pin LOW
} RobotState_t;



typedef enum {
    MSG_TYPE_NAV,
    MSG_TYPE_PID_TUNING
} MsgType_t;

typedef struct {
    float kp;
    float ki;
    float kd;
} PidTune_t;

/* ==========================================
   3. STRUCTS FOR INTER-TASK DATA FLOW
   ========================================== */
// Navigation inputs coming down from the Pi via CAN
typedef struct {
    float target_velocity;  // Linear speed (m/s)
    float target_yaw_rate;  // Turning rate (rad/s)
} NavCommand_t;

typedef struct {
    MsgType_t type;
    union {
        NavCommand_t nav;     // Existing navigation commands
        PidTune_t    tune;    // New PID tuning gains
    } payload;
} ControlCommand_t;

// Raw metrics packaged by Safety for transmission to Pi
typedef struct {
    uint8_t error_mask;     // The system_errors byte
    uint16_t left_us_dist;  // cm
    uint16_t right_us_dist; // cm
} TelemetryData_t;



/* ==========================================
   4. GLOBAL OS RESOURCE HANDLES (Extern declaration)
   ========================================== */
extern QueueHandle_t NavigationQueue;
extern QueueHandle_t TelemetryQueue;

extern osThreadId_t controlTaskHandle;
extern osThreadId_t safetyTaskHandle;
extern osThreadId_t commsTaskHandle;

// Shared global atomic states (accessed via thread-safe rules)
extern volatile uint8_t system_errors;
extern volatile RobotState_t current_robot_state;


#endif /* INC_FREERTOS_TASKS_H_ */
