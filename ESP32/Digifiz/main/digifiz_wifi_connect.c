/* 
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
// #include <esp_log.h>

#include "nvs_wifi_connect.h"

void digifiz_ws_connect(void);
esp_err_t digifiz_register_uri_handler(httpd_handle_t server);

//#define MDNS
#ifdef MDNS
#include "mdns.h"
#include "lwip/apps/netbiosns.h"
#endif // MDNS
#ifdef MDNS
static void initialise_mdns(void)
{
    mdns_init();
    mdns_hostname_set("esp");
    mdns_instance_name_set("esp home web server");

    mdns_txt_item_t serviceTxtData[] = {
        {"board", "esp32"},
        {"path", "/"}};

    ESP_ERROR_CHECK(mdns_service_add("ESP32-WebServer", "_http", "_tcp", 80, serviceTxtData,
                                     sizeof(serviceTxtData) / sizeof(serviceTxtData[0])));
}
#endif // MDNS

void digifiz_wifi_connect(void)
{
    esp_err_t ret = nvs_wifi_connect(); // return with error ?
#ifdef MDNS
    initialise_mdns();
    netbiosns_init();
    netbiosns_set_name("esp");
#endif // MDNS
#ifdef RUN_EXAMPLE_SERVER
    if (ret) // err nvs_wifi_connect start nvs_wifi_connect_start_http_server and restart
    {
        nvs_wifi_connect_start_http_server(NVS_WIFI_CONNECT_MODE_RESTART_ESP32, digifiz_register_uri_handler); // run server
    }
    else // run example server with nvs_wifi_connect_register_uri_handler(server);
    {
        digifiz_ws_connect();
    }
#else
    if (ret) // err nvs_wifi_connect start nvs_wifi_connect_start_http_server and restart
    {
        nvs_wifi_connect_start_http_server(NVS_WIFI_CONNECT_MODE_RESTART_ESP32, digifiz_register_uri_handler); // run server
    }
    else // run nvs_wifi_connect_start_http_server with register digifiz_register_uri_handler and stay active server
    {
        nvs_wifi_connect_start_http_server(NVS_WIFI_CONNECT_MODE_STAY_ACTIVE, digifiz_register_uri_handler); // run server
    }
#endif
}
