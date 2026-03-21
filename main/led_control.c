#include "led_control.h"
#include "driver/pwm.h"
#include <math.h>

// In ESP8266_RTOS_SDK, max duty cycle is dependent on the period:
// Max Duty = (Period * 1000) / 45
#define MAX_DUTY ((PWM_PERIOD * 1000) / 45)

static const uint32_t pwm_pins[4] = {PIN_RED, PIN_GREEN, PIN_BLUE, PIN_WHITE};
static uint32_t pwm_duties[4] = {0, 0, 0, 0};
static float pwm_phases[4] = {0, 0, 0, 0};

void led_control_init(void) {
  // Initialize PWM on the 4 RGBW channels
  pwm_init(PWM_PERIOD, pwm_duties, 4, pwm_pins);
  pwm_set_phases(pwm_phases);
  pwm_start();
}

// Convert HSV to RGBW and update PWM
void led_control_set(bool power, float hue, float sat, float bri) {
  if (!power) {
    for (int i = 0; i < 4; i++)
      pwm_set_duty(i, 0);
    pwm_start();
    return;
  }

  float s = sat / 100.0f;
  float v = bri / 100.0f;

  // HSV to RGB conversion
  float c = v * s;
  float x = c * (1.0f - fabs(fmod(hue / 60.0f, 2.0f) - 1.0f));
  float m = v - c;

  float r = 0, g = 0, b = 0;
  if (hue >= 0 && hue < 60) {
    r = c;
    g = x;
    b = 0;
  } else if (hue >= 60 && hue < 120) {
    r = x;
    g = c;
    b = 0;
  } else if (hue >= 120 && hue < 180) {
    r = 0;
    g = c;
    b = x;
  } else if (hue >= 180 && hue < 240) {
    r = 0;
    g = x;
    b = c;
  } else if (hue >= 240 && hue < 300) {
    r = x;
    g = 0;
    b = c;
  } else {
    r = c;
    g = 0;
    b = x;
  }

  r = r + m;
  g = g + m;
  b = b + m;

  // White channel mixes in when Saturation is low
  float w = (1.0f - s) * v;

  // Map 0.0-1.0 to ESP8266 PWM Duty Range
  pwm_set_duty(0, (uint32_t)(r * MAX_DUTY)); // Red
  pwm_set_duty(1, (uint32_t)(g * MAX_DUTY)); // Green
  pwm_set_duty(2, (uint32_t)(b * MAX_DUTY)); // Blue
  pwm_set_duty(3, (uint32_t)(w * MAX_DUTY)); // White

  pwm_start(); // Apply updated duties
}