#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <stdio.h>

static const char *TAG = "MAIN";

#include "homekit_config.h"
#include "ir_control.h"
#include "led_control.h"

#ifndef WIFI_SSID
#define WIFI_SSID "REPLACE_WITH_YOUR_SSID"
#endif

#ifndef WIFI_PASS
#define WIFI_PASS "REPLACE_WITH_YOUR_PASSWORD"
#endif

// Dispatch IR commands into HomeKit synchronized state changes
static void handle_ir_action(ir_action_t action) {
  switch (action) {
  case IR_ACTION_TOGGLE:
    hk_sync_toggle();
    break;
  case IR_ACTION_BRI_UP:
    hk_sync_brightness(10);
    break;
  case IR_ACTION_BRI_DOWN:
    hk_sync_brightness(-10);
    break;
  case IR_ACTION_COLOR_RED:
    hk_sync_color(0.0f, 100.0f);
    break;
  case IR_ACTION_COLOR_GREEN:
    hk_sync_color(120.0f, 100.0f);
    break;
  case IR_ACTION_COLOR_BLUE:
    hk_sync_color(240.0f, 100.0f);
    break;
  case IR_ACTION_COLOR_WHITE:
    hk_sync_color(0.0f, 0.0f);
    break;
  default:
    break;
  }
}

#include "esp_event_loop.h"

// Basic Wi-Fi Initialization
static esp_err_t event_handler(void *ctx, system_event_t *event) {
  switch (event->event_id) {
  case SYSTEM_EVENT_STA_START:
    ESP_LOGI(TAG, "Wi-Fi started, connecting...");
    esp_wifi_connect();
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    ESP_LOGI(TAG, "Wi-Fi connected! IP acquired.");
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    ESP_LOGI(TAG, "Wi-Fi disconnected. Reconnecting...");
    esp_wifi_connect();
    break;
  default:
    break;
  }
  return ESP_OK;
}

static void wifi_init(void) {
  tcpip_adapter_init();
  esp_event_loop_init(event_handler, NULL);
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);

  wifi_config_t wifi_config = {
      .sta =
          {
              .ssid = WIFI_SSID,
              .password = WIFI_PASS,
          },
  };

  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
  esp_wifi_start();
  esp_wifi_connect();
}

#include "driver/uart.h"

void app_main(void) {
  // macOS drivers cannot read 74880 baud. Force it to 115200 immediately.
  uart_set_baudrate(UART_NUM_0, 115200);

  // 1. Initialize NVS (Required by HomeKit & WiFi)
  ESP_LOGI(TAG, "Initializing NVS...");
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_LOGW(TAG, "NVS flash erase required.");
    nvs_flash_erase();
    nvs_flash_init();
  }

  // 2. Initialize Hardware Peripherals
  ESP_LOGI(TAG, "Initializing Hardware...");
  led_control_init();
  ir_control_init(handle_ir_action);

  // 3. Start Wi-Fi
  ESP_LOGI(TAG, "Starting Wi-Fi...");
  wifi_init();

  // 4. Start HomeKit Service
  ESP_LOGI(TAG, "Starting HomeKit...");
  homekit_setup();

  ESP_LOGI(TAG, "MagicHome ESP8285 Controller Initialized.");
  ESP_LOGI(TAG, "HomeKit Setup Code: 111-22-333");

  fflush(stdout);
}