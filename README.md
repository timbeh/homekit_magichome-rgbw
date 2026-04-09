# HomeKit MagicHome RGBW

ESP8266/ESP8285 firmware that bridges common RGBW + IR LED controllers (often running the MagicHome Software from factory) to Apple HomeKit. Control your RGB+White LED strips with the Home app, Siri, or an IR remote — with full state synchronization between both.

## Features

- **Apple HomeKit** lightbulb accessory with on/off, brightness, hue, and saturation
- **IR remote control** via NEC protocol (compatible with standard IR LED remotes)
- **Bidirectional sync**: IR commands update HomeKit state; HomeKit commands reflect in IR-controllable state
- **RGBW color mixing**: automatic white channel blending based on saturation
- 4-channel PWM LED output at 1 kHz

## Hardware

### Pin Assignments

| Function | GPIO |
|----------|------|
| Red PWM  | 5    |
| Green PWM | 12  |
| Blue PWM | 13   |
| White PWM | 15  |
| IR Receiver | 4  |

## Prerequisites

- [ESP-IDF](https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/get-started/index.html) for ESP8266 (RTOS SDK)
- CMake build system
- `esptool.py` for flashing

## Building

Set your WiFi credentials as environment variables, then build:

```bash
export WIFI_SSID="YourNetworkName"
export WIFI_PASS="YourPassword"

idf.py build
```

## Flashing

```bash
idf.py -p /dev/ttyUSB0 flash
```

Adjust the port (`/dev/ttyUSB0`) to match your system. On macOS this is typically `/dev/cu.usbserial-*` or similar.

## HomeKit Pairing

1. Open the **Home** app on iOS or macOS
2. Tap **Add Accessory** and choose **More options**
3. Select **MagicHome RGBW** from the list
4. Enter the setup code: **111-22-333**

The accessory appears as a lightbulb and supports on/off, brightness (10–100%), hue, and saturation.

## IR Remote

The firmware decodes NEC protocol IR codes. Default mappings:

| Button | NEC Code |
|--------|----------|
| On | `0xF7C03F` |
| Off | `0xF740BF` |
| Brightness Up | `0xF700FF` |
| Brightness Down | `0xF7807F` |
| Red | `0xF720DF` |
| Green | `0xF7A05F` |
| Blue | `0xF7609F` |
| White | `0xF7E01F` |

To use a different remote, update the `IR_CODE_*` constants in `main/ir_control.c`.

## Project Structure

```
├── main/
│   ├── main.c             # Entry point: WiFi, HomeKit, and IR initialization
│   ├── led_control.c/h    # 4-channel PWM driver and HSV→RGBW conversion
│   ├── ir_control.c/h     # NEC IR decoder (ISR + FreeRTOS task)
│   ├── homekit_config.c/h # HomeKit accessory definition and characteristic callbacks
│   └── CMakeLists.txt
├── CMakeLists.txt
└── sdkconfig
```

## Configuration

| Parameter | Location | Default |
|-----------|----------|---------|
| WiFi SSID | Environment variable | — |
| WiFi password | Environment variable | — |
| HomeKit setup code | `homekit_config.c` | `111-22-333` |
| Minimum brightness | `homekit_config.c` | 10% |
| IR codes | `ir_control.c` | See table above |
