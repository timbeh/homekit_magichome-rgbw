#include "homekit_config.h"
#include "led_control.h"
#include <stddef.h>
#include <homekit/characteristics.h>
#include <homekit/homekit.h>

// Current State
static bool led_on = false;
static float led_hue = 0.0f;   // 0-360
static float led_sat = 0.0f;   // 0-100
static float led_bri = 100.0f; // 0-100

// Characteristics Definitions
homekit_characteristic_t cha_on = HOMEKIT_CHARACTERISTIC_(ON, false);
homekit_characteristic_t cha_bri = HOMEKIT_CHARACTERISTIC_(BRIGHTNESS, 100);
homekit_characteristic_t cha_hue = HOMEKIT_CHARACTERISTIC_(HUE, 0.0f);
homekit_characteristic_t cha_sat = HOMEKIT_CHARACTERISTIC_(SATURATION, 0.0f);

static void update_leds(void) {
  led_control_set(led_on, led_hue, led_sat, led_bri);
}

// ---- HomeKit Setters ----
void hk_set_on(homekit_value_t value) {
  led_on = value.bool_value;
  update_leds();
}

void hk_set_bri(homekit_value_t value) {
  led_bri = value.int_value;
  update_leds();
}

void hk_set_hue(homekit_value_t value) {
  led_hue = value.float_value;
  update_leds();
}

void hk_set_sat(homekit_value_t value) {
  led_sat = value.float_value;
  update_leds();
}

// ---- Internal IR Synchronization ----
void hk_sync_toggle(void) {
  led_on = !led_on;
  cha_on.value.bool_value = led_on;
  homekit_characteristic_notify(&cha_on, cha_on.value);
  update_leds();
}

void hk_sync_brightness(int delta) {
  int new_bri = (int)led_bri + delta;
  if (new_bri > 100)
    new_bri = 100;
  if (new_bri < 10)
    new_bri = 10;

  led_bri = new_bri;
  cha_bri.value.int_value = new_bri;
  homekit_characteristic_notify(&cha_bri, cha_bri.value);
  update_leds();
}

void hk_sync_color(float hue, float sat) {
  led_hue = hue;
  led_sat = sat;

  cha_hue.value.float_value = led_hue;
  cha_sat.value.float_value = led_sat;

  homekit_characteristic_notify(&cha_hue, cha_hue.value);
  homekit_characteristic_notify(&cha_sat, cha_sat.value);

  if (!led_on)
    hk_sync_toggle(); // Turn on if off
  else
    update_leds();
}

// ---- Accessory Setup ----
homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(
            .id = 1, .category = homekit_accessory_category_lightbulb,
            .services =
                (homekit_service_t *[]){
                    HOMEKIT_SERVICE(
                        ACCESSORY_INFORMATION,
                        .characteristics =
                            (homekit_characteristic_t *[]){
                                HOMEKIT_CHARACTERISTIC(NAME, "MagicHome RGBW"),
                                HOMEKIT_CHARACTERISTIC(MANUFACTURER,
                                                       "Custom Firmware"),
                                HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "001"),
                                HOMEKIT_CHARACTERISTIC(MODEL,
                                                       "ESP8285 Controller"),
                                HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION,
                                                       "1.0"),
                                HOMEKIT_CHARACTERISTIC(IDENTIFY, NULL), NULL}),
                    HOMEKIT_SERVICE(
                        LIGHTBULB, .primary = true,
                        .characteristics =
                            (homekit_characteristic_t *[]){
                                &cha_on, &cha_bri, &cha_hue, &cha_sat, NULL}),
                    NULL}),
    NULL};

homekit_server_config_t config = {
    .accessories = accessories,
    .password = "111-22-333" // Setup Code for pairing
};

void homekit_setup(void) {
  cha_on.setter = hk_set_on;
  cha_bri.setter = hk_set_bri;
  cha_hue.setter = hk_set_hue;
  cha_sat.setter = hk_set_sat;
  homekit_server_init(&config);
}