idf_component_register(SRCS "ble_module.c" "gear_estimator.c" "kline.c" "mjs.c" "vehicle_data.c" "digifiz_time.c" "digifiz_wifi_connect.c" "digifiz_ws_server.c" "millis.c" "reg_inout.c" "frequency_count.c" "device_sleep.c" "emegrency.c" 
"buzzer.c" 
"protocol.c" 
"params.c" 
"mfa.c" 
"fuel_pressure.c" 
"adc.c" 
"display_next.c" 
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
data/digifiz_ws_connect.html
)
