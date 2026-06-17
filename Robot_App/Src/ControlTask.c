/*
 * ControlTask.c
 *
 * Created on: 5 Jun 2026
 * Author: whp27
 */

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"       // Required for osKernelGetTickCount and osDelayUntil
#include "freeRTOS_tasks.h"
#include "MPU6050_driver.h"
#include "pid_controller.h" // Links our dual-loop balance engine functions

/**
 * @brief Core real-time execution thread for the inverted pendulum balance engine.
 * Runs at a strict deterministic frequency of 250Hz (4ms loop time).
 */
void controlTask(void) {

    // 1. One-time hardware peripheral initializations
    MPU6050_INIT();

    // Explicitly seed our starting operational state
    current_robot_state = STATE_BALANCING;

    // Allocate local scratchpad memory for incoming network commands
    ControlCommand_t receivedData;

    // 2. Setup the deterministic 250Hz absolute execution window
    uint32_t tick_wake_time = osKernelGetTickCount();
    const uint32_t period_ms = 4;

    // 3. The Continuous Real-Time Loop Engine
    for (;;) {

        // --- STEP A: NON-BLOCKING QUEUE CHECK ---
        // We poll the NavigationQueue with a timeout of 0.
        // If the Pi hasn't sent a message, we bypass this immediately without stalling.
        if (xQueueReceive(NavigationQueue, &receivedData, 0) == pdPASS) {

            // Forward the unified command structure directly to our PID manager.
            // It will handle whether this is a speed command or custom Kp, Ki, Kd gains.
            update_pid_gains_and_targets(&receivedData);
        }

        // --- STEP B: EXECUTE SYSTEM BEHAVIOR ENGINE ---
        // We invoke our math engine. The internal state checks (Braking, Isolating, Balancing)
        // happen inside this function to keep this loop execution path clean and brief.
        execute_balance_engine();

        // --- STEP C: ABSOLUTE TIME ENFORCEMENT ---
        // Suspends this task until exactly 4ms have passed since the start of the last wake window.
        // This eliminates timing jitter completely, even if SPI/I2C cycles take variable micro-seconds.
        osDelayUntil(&tick_wake_time, period_ms);
    }
}


you can use system_errors everywhere using |= and &= to add errors to the error byte
