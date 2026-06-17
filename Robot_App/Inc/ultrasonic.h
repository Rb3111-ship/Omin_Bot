#ifndef ULTRASONIC_H
#define ULTRASONIC_H

typedef struct{
	uint32_t start_time;
	uint32_t finish_time;

	uint32_t pulse_width;

	uint8_t calculation_complete;
}UltrasonicSensor_t;

extern volatile UltrasonicSensor_t g_frontSensor;
extern volatile UltrasonicSensor_t g_rearSensor;

void reset_EchoState();

void init_timer();
void Ultrasonic_TriggerFront(void);
void Ultrasonic_TriggerRear(void);
#endif
