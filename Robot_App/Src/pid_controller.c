#include "pid_controller.h"
#include <math.h>
#include <string.h>

#define PWM_CAP      999.0f
#define ANGLE_MAX_LIMIT 15.0f  // Protection cap: never lean more than 15 degrees

// --- Inner Loop (Pitch Balance) Constants ---
float active_Kp = 30.0f;
float active_Ki = 2.5f;
float active_Kd = 2.5f;
static float integrator_inner = 0.0f;
static float prev_error_inner = 0.0f;

// --- Outer Loop (Velocity Control) Constants ---
// These will be tuned so the outer loop doesn't fight the inner loop
static float outer_Kp = 1.8f;
static float outer_Ki = 0.15f;
static float integrator_outer = 0.0f;

// --- Time & Physical Constants ---
static const float timestep = 0.004f; // 250Hz / 4ms execution period
// JGA25-370 wheel dimensions & encoder resolution constants:
// Adjust these based on your exact gear ratio and wheel diameter (in meters)
static const float METERS_PER_TICK = 0.00015f;

// --- Inbound Targets from Pi ---
static float target_velocity_mps = 0.0f;
static float target_yaw_rate_rads = 0.0f;

void setConst(float kp, float ki, float kd) {
    active_Kp = kp;
    active_Ki = ki;
    active_Kd = kd;
}

void update_pid_gains_and_targets(ControlCommand_t *cmd) {
    if (cmd->type == MSG_TYPE_PID_TUNING) {
        active_Kp = cmd->payload.tune.kp;
        active_Ki = cmd->payload.tune.ki;
        active_Kd = cmd->payload.tune.kd;
    } else if (cmd->type == MSG_TYPE_NAV) {
        target_velocity_mps = cmd->payload.nav.target_velocity;
        target_yaw_rate_rads = cmd->payload.nav.target_yaw_rate;
    }
}

void execute_balance_engine(void) {
    // 1. Evaluate Supreme System State
    switch(current_robot_state) {
        case STATE_FAILSAFE_STAGE1_BRAKE:
            target_velocity_mps = 0.0f;
            target_yaw_rate_rads = 0.0f;
            break;
        case STATE_FAILSAFE_STAGE3_ISOLATE:
            HAL_GPIO_WritePin(GPIOC, MOTOR_STBY_Pin, GPIO_PIN_RESET);
            MOTOR_PWM_L->CCR1 = 0;
            MOTOR_PWM_R->CCR1 = 0;
            return;
        default:
            HAL_GPIO_WritePin(GPIOC, MOTOR_STBY_Pin, GPIO_PIN_SET);
            break;
    }

    // 2. Fetch Raw Hardware Encoder Ticks
    // Read the counter registers directly from our hardware timers
    int16_t left_ticks = (int16_t)__HAL_TIM_GET_COUNTER(&htim2);
    int16_t right_ticks = (int16_t)__HAL_TIM_GET_COUNTER(&htim3);

    // Reset hardware timers immediately to capture clean deltas next cycle
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    __HAL_TIM_SET_COUNTER(&htim3, 0);

    // 3. Compute Actual Linear Velocity (Average of both wheels)
    float left_velocity = (float)left_ticks * METERS_PER_TICK / timestep;
    float right_velocity = (float)right_ticks * METERS_PER_TICK / timestep;
    float actual_velocity_mps = (left_velocity + right_velocity) / 2.0f;

    // 4. --- OUTER VELOCITY PID LOOP MATH ---
    float velocity_error = target_velocity_mps - actual_velocity_mps;

    // Proportional Term
    float outer_p_out = outer_Kp * velocity_error;

    // Integral Term with Anti-Windup Clamping
    integrator_outer += (velocity_error * timestep);
    if (integrator_outer > 5.0f) integrator_outer = 5.0f;
    if (integrator_outer < -5.0f) integrator_outer = -5.0f;
    float outer_i_out = outer_Ki * integrator_outer;

    // Output of Outer Loop is the target pitch lean angle
    float calculated_target_angle = outer_p_out + outer_i_out;

    // Bound the lean angle so the robot never commands a suicidal tilt
    if (calculated_target_angle > ANGLE_MAX_LIMIT)   calculated_target_angle = ANGLE_MAX_LIMIT;
    if (calculated_target_angle < -ANGLE_MAX_LIMIT) calculated_target_angle = -ANGLE_MAX_LIMIT;

    // 5. --- INNER PITCH BALANCE PID LOOP MATH ---
    float current_tilt = getCurrentTilt();
    if (isnan(current_tilt) || current_tilt > 45.0f || current_tilt < -45.0f) {
        system_errors |= FAULT_IMU_TIMEOUT; // Set global atomic flag
        return;
    }

    float inner_error = calculated_target_angle - current_tilt;

    float p_out = active_Kp * inner_error;

    integrator_inner += (inner_error * timestep);
    float i_out = active_Ki * integrator_inner;

    float slope = (inner_error - prev_error_inner) / timestep;
    float d_out = active_Kd * slope;

    float balance_Output = p_out + i_out + d_out;
    prev_error_inner = inner_error;

    // 6. Hardware Direction and PWM Actuation
    if (balance_Output < 0) {
        HAL_GPIO_WritePin(GPIOC, MOTOR_L_IN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOC, MOTOR_L_IN2_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, MOTOR_R_IN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOC, MOTOR_R_IN2_Pin, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(GPIOC, MOTOR_L_IN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, MOTOR_L_IN2_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOC, MOTOR_R_IN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, MOTOR_R_IN2_Pin, GPIO_PIN_SET);
    }

    // Convert yaw rate command directly to steering wheel differential offsets
    float steering_offset = target_yaw_rate_rads * 150.0f; // Tunable scaling factor

    float motor_output_l = fabsf(balance_Output) + steering_offset;
    float motor_output_r = fabsf(balance_Output) - steering_offset;

    MOTOR_PWM_L->CCR1 = motor_output_l > PWM_CAP ? PWM_CAP : motor_output_l;
    MOTOR_PWM_R->CCR1 = motor_output_r > PWM_CAP ? PWM_CAP : motor_output_r;
}
