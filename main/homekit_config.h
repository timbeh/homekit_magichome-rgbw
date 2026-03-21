#ifndef HOMEKIT_CONFIG_H
#define HOMEKIT_CONFIG_H

#include <stdbool.h>

void homekit_setup(void);

// Sync functions called by IR to update HomeKit state & LEDs
void hk_sync_toggle(void);
void hk_sync_brightness(int delta);
void hk_sync_color(float hue, float sat);

#endif