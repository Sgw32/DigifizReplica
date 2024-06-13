[Ru](/README-RU.md)

| Supported Targets |
| ESP32 ESP32S3 ESP32C3 |
| ----------------- |

# ESP32 WiFi Connect to an access point or create an access point using SSID/Pass saved in NVS. Designed to work in a local network.
When you turn it on for the first time (there is no data in NVS), an access point is created with the parameters specified in menuconfig.
You can connect a web server with a setup page in which you select the WiFi STA/SoftAP operating mode and WiFi connection passwords.
Connection parameters are saved in NVS. After saving the settings, the web server can be left running for other applications, stopped, or restarted the ESP32. After a reboot, WiFi will connect with the parameters saved in NVS.
If the STA is unable to connect to the selected access point after several attempts to connect the ESP32, it will switch to SoftAP mode with the parameters saved in NVS. Parameters can be changed at any convenient time by starting the web server. If the system already has a web server with websocket support, you can register the configuration page handler handle on the already running server. You can connect a web server from a component and leave it running by connecting the handles of your pages.
  - Connects as a component to your program
  - Data is stored in NVS
  - Configuring WIFI parameters via WEB interface
  - First connection via softAP
  - Subsequent connections - according to data recorded in NVS
  - Interface -> nvs_wifi_connect.h
     - esp_err_t nvs_wifi_connect(void); - connecting to WiFi using parameters saved in NVS, returns an error if the connection is unsuccessful and creates SoftAP with default or SoftAP parameters saved in NVS
     - httpd_handle_t nvs_wifi_connect_start_http_server(int restart_mode , nvs_wifi_connect_register_uri_handler_t register_uri_handler); - starts a web server with a page for setting up WiFi parameters, returns the server handle or NULL if the web server could not be created.
       - int restart_mode - server operating mode
         - NVS_WIFI_CONNECT_MODE_STAY_ACTIVE - the server remains active after saving the parameters in NVS; in this mode, the web server can be used as the main program server by connecting your handlers
         - NVS_WIFI_CONNECT_MODE_STOP_SERVER - the server will be stopped after saving the parameters in NVS, you can start your own web server if necessary, the ESP32 will not be rebooted
         - NVS_WIFI_CONNECT_MODE_RESTART_ESP32 - after saving the parameters, the ESP32 will be rebooted
     - esp_err_t nvs_wifi_connect_register_uri_handler(httpd_handle_t server); - you can register an NVS configuration page handler on an existing web server, returns an error if the handlers could not be registered
       - httpd_handle_t server - server handle for connection
     - void nvs_wifi_connect_init_softap(char *ap_ssid, char *ap_pass); - creates SoftAP with parameters (ap_ssid/ap_pass) regardless of NVS data
     - esp_err_t nvs_wifi_connect_init_sta(char *sta_ssid, char *sta_pass); - connects to an existing access point with parameters (sta_ssid/sta_pass) regardless of NVS data. Returns an error if connection is not possible.
  - Example -> example_nvs_wifi_connect.c
  - menuconfig options
    - CONFIG_HTTPD_WS_SUPPORT=y
  