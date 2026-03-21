#ifndef IR_CONTROL_H
#define IR_CONTROL_H

#include <stdint.h>

#define PIN_IR 4

typedef enum {
  IR_ACTION_NONE,
  IR_ACTION_TOGGLE,
  IR_ACTION_BRI_UP,
  IR_ACTION_BRI_DOWN,
  IR_ACTION_COLOR_RED,
  IR_ACTION_COLOR_GREEN,
  IR_ACTION_COLOR_BLUE,
  IR_ACTION_COLOR_WHITE
} ir_action_t;

typedef void (*ir_callback_t)(ir_action_t action);

void ir_control_init(ir_callback_t cb);

#endif