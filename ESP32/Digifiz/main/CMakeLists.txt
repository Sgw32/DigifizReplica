cmake_minimum_required(VERSION 3.5)

find_program(GZIP_EXECUTABLE gzip)

set(WEB_FILES digifiz_ws_connect.html)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data)
set(DIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data/dist)

file(MAKE_DIRECTORY ${DIST_DIR})

foreach(file ${WEB_FILES})
    if(file MATCHES "\\.png$")
        list(APPEND OUTPUT_FILES ${DIST_DIR}/${file})
    else()
        list(APPEND OUTPUT_FILES ${DIST_DIR}/${file}.gz)
    endif()
endforeach()

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
    "main.c"
    "tacho.c"
    "speedometer.c"
    "oscilloscope.c"
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
        ${OUTPUT_FILES}
)

add_custom_target(frontend_assets ALL DEPENDS ${OUTPUT_FILES})

foreach(file ${WEB_FILES})
    if(file MATCHES "\\.png$")
        add_custom_command(
            OUTPUT ${DIST_DIR}/${file}
            COMMAND ${CMAKE_COMMAND} -E copy
                ${SRC_DIR}/${file}
                ${DIST_DIR}/${file}
            DEPENDS ${SRC_DIR}/${file}
            COMMENT "Copying ${file}"
        )
    else()
        add_custom_command(
            OUTPUT ${DIST_DIR}/${file}.gz
            COMMAND ${GZIP_EXECUTABLE} -k -9 -c
                ${SRC_DIR}/${file} >
                ${DIST_DIR}/${file}.gz
            DEPENDS ${SRC_DIR}/${file}
            COMMENT "Compressing ${file}"
        )
    endif()
endforeach()

add_dependencies(${COMPONENT_LIB} frontend_assets)
