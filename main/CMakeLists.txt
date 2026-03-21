# main/CMakeLists.txt
if(DEFINED ENV{WIFI_SSID})
    add_compile_definitions(WIFI_SSID=\"$ENV{WIFI_SSID}\")
endif()
if(DEFINED ENV{WIFI_PASS})
    add_compile_definitions(WIFI_PASS=\"$ENV{WIFI_PASS}\")
endif()

idf_component_register(SRCS "main.c" "led_control.c" "ir_control.c" "homekit_config.c"
                       INCLUDE_DIRS ".")