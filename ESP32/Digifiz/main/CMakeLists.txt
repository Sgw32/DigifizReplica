cmake_minimum_required(VERSION 3.5)

idf_build_get_property(python PYTHON)
set(WEB_SOURCE ${COMPONENT_DIR}/data/digifiz_ws_connect.html)
set(WEB_HTML ${CMAKE_CURRENT_BINARY_DIR}/web/digifiz_ws_connect.html)
set(WEB_GZIP ${WEB_HTML}.gz)
set(WEB_BUILDER ${COMPONENT_DIR}/tools/build_web_page.py)
file(STRINGS ${COMPONENT_DIR}/setup.h REFIZ_DEFINE
    REGEX "^#[ \t]*define[ \t]+DIGIFIZ_REFIZ_DISPLAY")
set(WEB_VARIANT next)
if(REFIZ_DEFINE)
    set(WEB_VARIANT refiz)
endif()

idf_component_register(
    SRCS "ble_module.c" "gear_estimator.c" "kline.c" "vehicle_data.c" "digifiz_time.c" "digifiz_wifi_connect.c" "digifiz_ws_server.c" "millis.c" "reg_inout.c" "frequency_count.c" "device_sleep.c" "emegrency.c"
    "buzzer.c"
    "protocol.c"
    "params.c"
    "mfa.c"
    "fuel_pressure.c"
    "adc.c"
    "led_effects.c"
    "display_next.c"
    "display_refiz.c"
    "main.c"
    "tacho.c"
    "speedometer.c"
    "kline.c"
    "gear_estimator.c"
    INCLUDE_DIRS "."
    REQUIRES
        nvs_wifi_connect
        xparam
        esp_http_server
        esp_wifi
        bt
        nvs_flash
        esp_adc
        json
        app_update
        driver
    EMBED_FILES
        ${WEB_GZIP}
)

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/setup.h)
add_custom_command(OUTPUT ${WEB_HTML} ${WEB_GZIP}
    COMMAND ${python} ${WEB_BUILDER} --source ${WEB_SOURCE} --output ${WEB_HTML} --variant ${WEB_VARIANT}
    DEPENDS ${WEB_SOURCE} ${WEB_BUILDER} ${CMAKE_CURRENT_SOURCE_DIR}/setup.h
    VERBATIM)
add_custom_target(frontend_assets ALL DEPENDS ${WEB_HTML} ${WEB_GZIP})

add_dependencies(${COMPONENT_LIB} frontend_assets)
