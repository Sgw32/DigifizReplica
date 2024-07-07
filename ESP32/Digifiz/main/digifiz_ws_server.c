/* 
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "digifiz_ws_server.h"

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_ota_ops.h>
#include "esp_netif.h"
#include <esp_http_server.h>
#include <cJSON.h>
#include "nvs_wifi_connect.h"
#include "protocol.h"
#include <sys/param.h>
#include <string.h>


/* A simple example that demonstrates using websocket echo server
 */
static const char *WS_TAG = "ws_echo_server";

esp_err_t digifiz_register_uri_handler(httpd_handle_t server);
esp_err_t update_post_handler(httpd_req_t *req);
static esp_err_t echo_handler(httpd_req_t *req);
/*
 * This handler echos back the received ws data
 * and triggers an async send if certain message received
 */
static esp_err_t echo_handler(httpd_req_t *req)
{
    if (req->method == HTTP_GET) {
        ESP_LOGI(WS_TAG, "Handshake done, the new connection was opened");
        return ESP_OK;
    }
    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    /* Set max_len = 0 to get the frame len */
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(WS_TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }
    //ESP_LOGI(TAG, "frame len is %d", ws_pkt.len);
    cJSON *json_send = cJSON_CreateObject(); 
    char *json_str = 0;
    if (ws_pkt.len) {
        /* ws_pkt.len + 1 is for NULL termination as we are expecting a string */
        buf = calloc(1, ws_pkt.len + 1);
        if (buf == NULL) {
            ESP_LOGE(WS_TAG, "Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        /* Set max_len = ws_pkt.len to get the frame payload */
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            ESP_LOGE(WS_TAG, "httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }
        ESP_LOGI(WS_TAG, "Got packet with message: %s", ws_pkt.payload);
        cJSON *json = cJSON_Parse((char*)ws_pkt.payload); 
        if (json == NULL) { 
            const char *error_ptr = cJSON_GetErrorPtr(); 
            if (error_ptr != NULL) { 
                printf("Error: %s\n", error_ptr); 
            } 
        } 
        else
        {
            // access the JSON data 
            cJSON *name = cJSON_GetObjectItemCaseSensitive(json, "name"); 
            if (cJSON_IsString(name) && (name->valuestring != NULL)) { 
                printf("Name: %s\n", name->valuestring); 
            } 
            cJSON *msg = cJSON_GetObjectItemCaseSensitive(json, "msg"); 
            if (cJSON_IsString(msg) && (msg->valuestring != NULL)) { 
                printf("Msg: %s\n", msg->valuestring); 
                protocolParse(msg->valuestring,strlen(msg->valuestring)); 
            } 
            
            cJSON_AddStringToObject(json_send, "name", "digifizReply"); 
            cJSON_AddStringToObject(json_send, "msg", ws_data_send); 

            json_str = cJSON_Print(json_send); 
            printf("json_str:%s\n",json_str);
            ws_pkt.payload = (uint8_t*)json_str;
            ws_pkt.len = strlen(json_str);
        }
        cJSON_Delete(json); 
    }
    //ESP_LOGI(TAG, "Packet type: %d", ws_pkt.type);
    ret = httpd_ws_send_frame(req, &ws_pkt);
    if (ret != ESP_OK) {
        ESP_LOGE(WS_TAG, "httpd_ws_send_frame failed with %d", ret);
    }
    free(buf);
    clearProtocolBuffer();
    if (json_str)
        cJSON_free(json_str); 
    cJSON_Delete(json_send); 
    return ret;
}

static const httpd_uri_t example_ws = {
        .uri        = "/ws",
        .method     = HTTP_GET,
        .handler    = echo_handler,
        .user_ctx   = NULL,
        .is_websocket = true
};

static esp_err_t get_handler(httpd_req_t *req)
{
    extern const unsigned char digifiz_ws_connect_html_start[] asm("_binary_digifiz_ws_connect_html_start");
    extern const unsigned char digifiz_ws_connect_html_end[] asm("_binary_digifiz_ws_connect_html_end");
    const size_t digifiz_ws_connect_html_size = (digifiz_ws_connect_html_end - digifiz_ws_connect_html_start);

    httpd_resp_send_chunk(req, (const char *)digifiz_ws_connect_html_start, digifiz_ws_connect_html_size);
    httpd_resp_sendstr_chunk(req, NULL);
    return ESP_OK;
}
static const httpd_uri_t example_gh = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = get_handler,
    .user_ctx = NULL};

httpd_uri_t update_post = {
	.uri	  = "/update",
	.method   = HTTP_POST,
	.handler  = update_post_handler,
	.user_ctx = NULL
};

/*
 * Handle OTA file upload
 */
esp_err_t update_post_handler(httpd_req_t *req)
{
	char buf[1000];
	esp_ota_handle_t ota_handle;
	int remaining = req->content_len;

	const esp_partition_t *ota_partition = esp_ota_get_next_update_partition(NULL);
	ESP_ERROR_CHECK(esp_ota_begin(ota_partition, OTA_SIZE_UNKNOWN, &ota_handle));

	while (remaining > 0) {
		int recv_len = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)));

		// Timeout Error: Just retry
		if (recv_len == HTTPD_SOCK_ERR_TIMEOUT) {
			continue;

		// Serious Error: Abort OTA
		} else if (recv_len <= 0) {
			httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Protocol Error");
			return ESP_FAIL;
		}

		// Successful Upload: Flash firmware chunk
		if (esp_ota_write(ota_handle, (const void *)buf, recv_len) != ESP_OK) {
			httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Flash Error");
			return ESP_FAIL;
		}

		remaining -= recv_len;
	}

	// Validate and switch to new OTA image and reboot
	if (esp_ota_end(ota_handle) != ESP_OK || esp_ota_set_boot_partition(ota_partition) != ESP_OK) {
			httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Validation / Activation Error");
			return ESP_FAIL;
	}

	httpd_resp_sendstr(req, "Firmware update complete, rebooting now!\n");

	vTaskDelay(500 / portTICK_PERIOD_MS);
	esp_restart();

	return ESP_OK;
}


static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(WS_TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Registering the ws handler
        ESP_LOGI(WS_TAG, "Registering URI handlers");
        digifiz_register_uri_handler(server);
        nvs_wifi_connect_register_uri_handler(server);
        return server;
    }

    ESP_LOGI(WS_TAG, "Error starting server!");
    return NULL;
}

static esp_err_t stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    return httpd_stop(server);
}

static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(WS_TAG, "Stopping webserver");
        if (stop_webserver(*server) == ESP_OK) {
            *server = NULL;
        } else {
            ESP_LOGE(WS_TAG, "Failed to stop http server");
        }
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(WS_TAG, "Starting webserver");
        *server = start_webserver();
    }
}


void digifiz_ws_connect(void)
{
    static httpd_handle_t server = NULL;
    initComProtocol();

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, &disconnect_handler, &server));

    /* Start the server for the first time */
    server = start_webserver();
}

esp_err_t digifiz_register_uri_handler(httpd_handle_t server)
{
    esp_err_t ret = ESP_OK;
    ret = httpd_register_uri_handler(server, &example_gh);
    if (ret)
        goto _ret;
    ret = httpd_register_uri_handler(server, &example_ws);
    if (ret)
        goto _ret;
    ret = httpd_register_uri_handler(server, &update_post);
_ret:
    return ret;
}

