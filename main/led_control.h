#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <stdbool.h>
#include <stdint.h>

// Standard MagicHome Pinout
#define PIN_RED 5
#define PIN_GREEN 12
#define PIN_BLUE 13
#define PIN_WHITE 15

// 1kHz PWM period (1000us)
#define PWM_PERIOD 1000

void led_control_init(void);
void led_control_set(bool power, float hue, float sat, float bri);

#endif