#include "ir_control.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

static QueueHandle_t ir_queue;
static ir_callback_t app_ir_cb = NULL;

// Example MagicHome NEC Codes (You may need to adjust these for your specific
// remote)
#define IR_CODE_ON 0xF7C03F
#define IR_CODE_OFF 0xF740BF
#define IR_CODE_BRI_UP 0xF700FF
#define IR_CODE_BRI_DOWN 0xF7807F
#define IR_CODE_RED 0xF720DF
#define IR_CODE_GREEN 0xF7A05F
#define IR_CODE_BLUE 0xF7609F
#define IR_CODE_WHITE 0xF7E01F

static void IRAM_ATTR ir_isr_handler(void *arg) {
  static uint32_t last_time = 0;
  static uint32_t ir_data = 0;
  static int bit_count = 0;
  static bool receiving = false;

  uint32_t now = esp_timer_get_time();
  uint32_t delta = now - last_time;
  last_time = now;

  // NEC Protocol Falling-to-Falling Edge Decoding
  if (delta > 13000 && delta < 14000) {
    // Header detected (~13.5ms total)
    ir_data = 0;
    bit_count = 0;
    receiving = true;
  } else if (receiving) {
    if (delta > 1000 && delta < 1300) {
      // Logic '0' (~1.12ms)
      ir_data >>= 1;
      bit_count++;
    } else if (delta > 2000 && delta < 2500) {
      // Logic '1' (~2.25ms)
      ir_data >>= 1;
      ir_data |= 0x80000000;
      bit_count++;
    } else {
      receiving = false; // Noise or error
    }

    if (bit_count == 32) {
      receiving = false;
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      xQueueSendFromISR(ir_queue, &ir_data, &xHigherPriorityTaskWoken);
      if (xHigherPriorityTaskWoken)
        portYIELD_FROM_ISR();
    }
  }
}

static void ir_task(void *pvParameters) {
  uint32_t ir_code;
  while (1) {
    if (xQueueReceive(ir_queue, &ir_code, portMAX_DELAY)) {
      ir_action_t action = IR_ACTION_NONE;

      // Map Code to Action
      switch (ir_code) {
      case IR_CODE_ON:
      case IR_CODE_OFF:
        action = IR_ACTION_TOGGLE;
        break;
      case IR_CODE_BRI_UP:
        action = IR_ACTION_BRI_UP;
        break;
      case IR_CODE_BRI_DOWN:
        action = IR_ACTION_BRI_DOWN;
        break;
      case IR_CODE_RED:
        action = IR_ACTION_COLOR_RED;
        break;
      case IR_CODE_GREEN:
        action = IR_ACTION_COLOR_GREEN;
        break;
      case IR_CODE_BLUE:
        action = IR_ACTION_COLOR_BLUE;
        break;
      case IR_CODE_WHITE:
        action = IR_ACTION_COLOR_WHITE;
        break;
      default:
        break;
      }

      if (action != IR_ACTION_NONE && app_ir_cb) {
        app_ir_cb(action);
      }
    }
  }
}

void ir_control_init(ir_callback_t cb) {
  app_ir_cb = cb;
  ir_queue = xQueueCreate(10, sizeof(uint32_t));

  gpio_config_t io_conf = {.intr_type =
                               GPIO_INTR_NEGEDGE, // Trigger on falling edge
                           .pin_bit_mask = (1ULL << PIN_IR),
                           .mode = GPIO_MODE_INPUT,
                           .pull_up_en = GPIO_PULLUP_ENABLE};
  gpio_config(&io_conf);

  gpio_install_isr_service(0);
  gpio_isr_handler_add(PIN_IR, ir_isr_handler, NULL);

  xTaskCreate(ir_task, "ir_task", 2048, NULL, 10, NULL);
}