#pragma once

#include <esp_http_server.h>


enum {
    NVS_WIFI_CONNECT_MODE_STAY_ACTIVE, // -> no operation
    NVS_WIFI_CONNECT_MODE_STOP_SERVER, // -> stop httpd
    NVS_WIFI_CONNECT_MODE_RESTART_ESP32 // -> full restart esp32 regardless of value NVS_WIFI_RESTART_VALUE_RESTART
};

#ifdef __cplusplus
extern "C"
{
#endif

/*
*   @brief  connect to wifi
*           connect mode ( ap/sta , ssid/pass ) read from NVS
*   @return
*           ESP_OK      -> mode read from NVS & connect to wifi OK
*           ESP_FAIL    -> can`t connect to wifi with NVS data, create AP  with CONFIG_DEFAULT_AP_ESP_WIFI_SSID/CONFIG_DEFAULT_AP_ESP_WIFI_PASS
*/
esp_err_t nvs_wifi_connect(void);
/*
*   @brief  create softAP with ap_ssid/ap_pass ip=192.168.4.1
*   @param  char *ap_ssid -> softAP ssid
*   @param  char *ap_pass -> softAP pass -> NULL -> authmode = WIFI_AUTH_OPEN
*   @return
*           ESP_OK              -> connect to wifi OK
*           ESP_ERR_INVALID_ARG -> invalid ssid/pass. Create default AP
*           ESP_FAIL            -> can`t connect to wifi or create AP

*/
void nvs_wifi_connect_init_softap(char *ap_ssid, char *ap_pass);
/*
*   @brief  connect to WIFI with sta_ssid/sta_pass 
*   @param  char *sta_ssid -> WIFI ssid
*   @param  char *sta_pass -> WIFI pass
*   @return
*           ESP_OK              -> connect to wifi OK
*           ESP_FAIL            -> can`t connect to wifi with ssid/pass
*/
esp_err_t nvs_wifi_connect_init_sta(char *sta_ssid, char *sta_pass);
/*
*   @brief  register nvs_wifi_connect handlers ( web page & ws handlers) on existing  httpd server with ws support
*           uri page -> CONFIG_DEFAULT_NVS_WIFI_CONNECT_URI
*   @param  httpd_handle_t server -> existing server handle
*   @return
*           ESP_OK      -> register OK
*           ESP_FAIL    -> register FAIL
*/
esp_err_t nvs_wifi_connect_register_uri_handler(httpd_handle_t server);

/*
*   @brief handle for connecting to the nvs_wifi_connect_http_server server
*/
typedef esp_err_t (*nvs_wifi_connect_register_uri_handler_t)(httpd_handle_t server);

/*
*   @brief  start nvs_wifi_connect httpd server, uri web page read existing nvs wifi data  & write new nvs wifi data ( ap/sta mode, wifi ssid/pass )
*           uri page -> CONFIG_DEFAULT_NVS_WIFI_CONNECT_URI
*   @param int restart_mode
*           NVS_WIFI_CONNECT_MODE_STAY_ACTIVE   -> write nvs wifi data and stay active connection
*           NVS_WIFI_CONNECT_MODE_STOP_SERVER   -> stop httpd after write new nvs wifi data
*           NVS_WIFI_CONNECT_MODE_RESTART_ESP32 -> full restart esp32 regardless of value NVS_WIFI_RESTART_VALUE_RESTART
*   @param  nvs_wifi_connect_register_uri_handler_t register_uri_handle
*           connect other uri handlers with started httpd server
*   @return  
*           httpd_handle_t server -> server handle on OK start
*           NULL                  -> server start FAIL
*/
httpd_handle_t nvs_wifi_connect_start_http_server(int restart_mode , nvs_wifi_connect_register_uri_handler_t register_uri_handler);

/*
*   @brief  Enable/disable WiFi auto-shutdown feature
*   @param  bool enable - true to enable auto-shutdown, false to disable
*/
void nvs_wifi_set_auto_shutdown(bool enable);

/*
*   @brief  Get current state of WiFi auto-shutdown feature
*   @return bool - true if auto-shutdown is enabled, false if disabled
*/
bool nvs_wifi_get_auto_shutdown(void);

/*
*   @brief  Reset the WiFi auto-shutdown timer
*           If auto-shutdown is enabled, this will restart WiFi and the shutdown timer
*/
void nvs_wifi_reset_timer(void);

#ifdef __cplusplus
}
#endif