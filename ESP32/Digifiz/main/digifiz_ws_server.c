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
#include "params.h"
#include "oscilloscope.h"
#include <sys/param.h>
#include <string.h>
#include "display_next.h"


/* A simple example that demonstrates using websocket echo server
 */
static const char *WS_TAG = "ws_echo_server";

esp_err_t digifiz_register_uri_handler(httpd_handle_t server);
esp_err_t update_post_handler(httpd_req_t *req);
static esp_err_t echo_handler(httpd_req_t *req);
static esp_err_t params_get_handler(httpd_req_t *req);
static esp_err_t oscilloscope_dump_handler(httpd_req_t *req);
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
                //printf("Name: %s\n", name->valuestring); 
            } 
            cJSON *msg = cJSON_GetObjectItemCaseSensitive(json, "msg"); 
            if (cJSON_IsString(msg) && (msg->valuestring != NULL)) { 
                //printf("Msg: %s\n", msg->valuestring); 
                protocolParse(msg->valuestring,strlen(msg->valuestring)); 
            } 
            
            cJSON_AddStringToObject(json_send, "name", "digifizReply"); 
            cJSON_AddStringToObject(json_send, "msg", ws_data_send); 

            json_str = cJSON_Print(json_send); 
            //printf("json_str:%s\n",json_str);
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

static esp_err_t set_param_post_handler(httpd_req_t *req)
{
    char buf[256];
    int ret, remaining = req->content_len;

    if (remaining >= sizeof(buf)) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Request body too large");
        return ESP_FAIL;
    }

    ret = httpd_req_recv(req, buf, remaining);
    if (ret <= 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Failed to receive data");
        return ESP_FAIL;
    }
    buf[ret] = '\0'; // Null-terminate

    cJSON *json = cJSON_Parse(buf);
    if (!json) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }

    cJSON *name_item = cJSON_GetObjectItemCaseSensitive(json, "name");
    cJSON *value_item = cJSON_GetObjectItemCaseSensitive(json, "value");

    if (!cJSON_IsString(name_item) || name_item->valuestring == NULL || value_item == NULL) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing or invalid 'name' or 'value'");
        return ESP_FAIL;
    }

    extern xparam_table_t params_table;
    xparam_t* param = xparam_find_by_name(&params_table, name_item->valuestring);
    if (!param) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Parameter not found");
        return ESP_FAIL;
    }

    // Ignore string parameters
    if (param->value_type == XPARAM_STRING) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Setting string parameters is not supported");
        return ESP_FAIL;
    }

    // Convert value -> uint32_t for passing into xparam_set_value
    uint32_t uval = 0;

    if (cJSON_IsNumber(value_item)) {
        // float value must be bit-cast if type is float
        if (param->value_type == XPARAM_FLOAT) {
            union {
                float f;
                uint32_t u32;
            } u;
            u.f = (float)value_item->valuedouble;
            uval = u.u32;
        } else {
            uval = (uint32_t)value_item->valuedouble;
        }
    } else if (cJSON_IsString(value_item)) {
        // Try to parse string as a number
        char *endptr;
        double f = strtod(value_item->valuestring, &endptr);
        if (*endptr != '\0') {
            cJSON_Delete(json);
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid number string");
            return ESP_FAIL;
        }

        if (param->value_type == XPARAM_FLOAT) {
            union {
                float f;
                uint32_t u32;
            } u;
            u.f = (float)f;
            uval = u.u32;
        } else {
            uval = (uint32_t)f;
        }
    } else {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Unsupported value type");
        return ESP_FAIL;
    }

    uint8_t result = xparam_set_value(param, uval);
    cJSON_Delete(json);

    if (result == 1) {
        httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
        return ESP_OK;
    } else {
        //result = 0
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to set parameter");
        return ESP_FAIL;
    }
}

httpd_uri_t set_param_post = {
    .uri      = "/set_param",
    .method   = HTTP_POST,
    .handler  = set_param_post_handler,
    .user_ctx = NULL
};

static const httpd_uri_t example_ws = {
        .uri        = "/ws",
        .method     = HTTP_GET,
        .handler    = echo_handler,
        .user_ctx   = NULL,
        .is_websocket = true
};

static esp_err_t params_get_handler(httpd_req_t *req)
{
    char query[100];
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        char type[16];
        if (httpd_query_key_value(query, "type", type, sizeof(type)) == ESP_OK) {
            if (strcmp(type, "params") == 0) {
                extern xparam_table_t params_table;
                char *json_str = xparam_table_to_json(&params_table);
                if (!json_str) {
                    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create JSON");
                    return ESP_FAIL;
                }
                httpd_resp_set_type(req, "application/json");
                httpd_resp_sendstr(req, json_str);
                free(json_str);
                return ESP_OK;
            } else if (strcmp(type, "color") == 0) {
                cJSON *root = cJSON_CreateObject();
                cJSON *scheme = cJSON_CreateArray();
                cJSON_AddItemToObject(root, "scheme", scheme);
                for (int i = 0; i < COLORING_SCHEME_MAX_ELEMENTS; i++) {
                    cJSON *item = cJSON_CreateObject();
                    cJSON_AddNumberToObject(item, "end_segment", digifizCustom.scheme[i].end_segment);
                    cJSON_AddNumberToObject(item, "type", digifizCustom.scheme[i].type);
                    cJSON_AddNumberToObject(item, "basecolor_enabled", digifizCustom.scheme[i].basecolor_enabled);
                    cJSON_AddNumberToObject(item, "r", digifizCustom.scheme[i].r);
                    cJSON_AddNumberToObject(item, "g", digifizCustom.scheme[i].g);
                    cJSON_AddNumberToObject(item, "b", digifizCustom.scheme[i].b);
                    cJSON_AddItemToArray(scheme, item);
                }
                char *json_str = cJSON_Print(root);
                httpd_resp_set_type(req, "application/json");
                httpd_resp_sendstr(req, json_str);
                cJSON_Delete(root);
                cJSON_free(json_str);
                return ESP_OK;
            }
        }
    }

    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing or invalid 'type' parameter");
    return ESP_FAIL;
}

static const httpd_uri_t params_get = {
    .uri      = "/params",
    .method   = HTTP_GET,
    .handler  = params_get_handler,
    .user_ctx = NULL
};

static const httpd_uri_t oscilloscope_dump = {
    .uri      = "/oscilloscope_dump",
    .method   = HTTP_GET,
    .handler  = oscilloscope_dump_handler,
    .user_ctx = NULL
};

static esp_err_t oscilloscope_dump_handler(httpd_req_t *req)
{
    uint8_t *payload = NULL;
    size_t payload_size = 0;
    esp_err_t ret = oscilloscope_build_dump(&payload, &payload_size);
    if (ret != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to build oscilloscope dump");
        return ret;
    }

    httpd_resp_set_type(req, "application/octet-stream");
    httpd_resp_set_hdr(req, "Content-Disposition", "attachment; filename=oscilloscope_dump.bin");
    ret = httpd_resp_send(req, (const char *)payload, payload_size);
    free(payload);
    return ret;
}


static esp_err_t get_handler(httpd_req_t *req)
{
    extern const unsigned char digifiz_ws_connect_html_gz_start[] asm("_binary_digifiz_ws_connect_html_gz_start");
    extern const unsigned char digifiz_ws_connect_html_gz_end[] asm("_binary_digifiz_ws_connect_html_gz_end");
    const size_t digifiz_ws_connect_html_gz_size = (digifiz_ws_connect_html_gz_end - digifiz_ws_connect_html_gz_start);

    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    httpd_resp_send(req, (const char *)digifiz_ws_connect_html_gz_start, digifiz_ws_connect_html_gz_size);
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

uint8_t flag_update_active = 0;

uint8_t get_update_in_progress(void)
{
    return flag_update_active;
}

/*
 * Handle OTA file upload
 */
esp_err_t update_post_handler(httpd_req_t *req)
{
	char buf[1000];
	esp_ota_handle_t ota_handle;
	int remaining = req->content_len;
    //digifiz_parameters.brightnessLevel = 1;
	const esp_partition_t *ota_partition = esp_ota_get_next_update_partition(NULL);
	ESP_ERROR_CHECK(esp_ota_begin(ota_partition, OTA_SIZE_UNKNOWN, &ota_handle));
    flag_update_active = 1;
	while (remaining > 0) {
		int recv_len = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)));

		// Timeout Error: Just retry
		if (recv_len == HTTPD_SOCK_ERR_TIMEOUT) {
			continue;

		// Serious Error: Abort OTA
		} else if (recv_len <= 0) {
			httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Protocol Error");
            flag_update_active = 0;
			return ESP_FAIL;
		}

		// Successful Upload: Flash firmware chunk
		if (esp_ota_write(ota_handle, (const void *)buf, recv_len) != ESP_OK) {
			httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Flash Error");
            flag_update_active = 0;
			return ESP_FAIL;
		}

		remaining -= recv_len;
	}

	// Validate and switch to new OTA image and reboot
	if (esp_ota_end(ota_handle) != ESP_OK || esp_ota_set_boot_partition(ota_partition) != ESP_OK) {
			httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Validation / Activation Error");
            flag_update_active = 0;
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
    config.stack_size = 20480; 
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

static esp_err_t set_color_scheme_handler(httpd_req_t *req)
{
    char buf[512];
    int ret, remaining = req->content_len;

    if (remaining >= sizeof(buf)) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Request body too large");
        return ESP_FAIL;
    }

    ret = httpd_req_recv(req, buf, remaining);
    if (ret <= 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Failed to receive data");
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    cJSON *json = cJSON_Parse(buf);
    if (!json) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }

    cJSON *scheme = cJSON_GetObjectItemCaseSensitive(json, "scheme");
    if (!cJSON_IsArray(scheme)) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing or invalid 'scheme' array");
        return ESP_FAIL;
    }

    int i = 0;
    cJSON *item = NULL;
    cJSON_ArrayForEach(item, scheme) {
        cJSON *end_segment = cJSON_GetObjectItemCaseSensitive(item, "end_segment");
        cJSON *type = cJSON_GetObjectItemCaseSensitive(item, "type");
        cJSON *basecolor_enabled = cJSON_GetObjectItemCaseSensitive(item, "basecolor_enabled");
        cJSON *r = cJSON_GetObjectItemCaseSensitive(item, "r");
        cJSON *g = cJSON_GetObjectItemCaseSensitive(item, "g");
        cJSON *b = cJSON_GetObjectItemCaseSensitive(item, "b");
        cJSON *i_data = cJSON_GetObjectItemCaseSensitive(item, "i");

        if (!i_data || !cJSON_IsNumber(i_data)) {
            ESP_LOGE(WS_TAG, "Missing or invalid index 'i'");
            cJSON_Delete(json);
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing or invalid 'i'");
            return ESP_FAIL;
        }

        i = i_data->valueint;
        ESP_LOGI(WS_TAG, "Processing scheme index: %d", i);

        if (i < 0) i = 0;
        if (i >= COLORING_SCHEME_MAX_ELEMENTS) i = COLORING_SCHEME_MAX_ELEMENTS - 1;

        if (!cJSON_IsNumber(end_segment) || !cJSON_IsNumber(type) ||
            !cJSON_IsNumber(basecolor_enabled) || !cJSON_IsNumber(r) ||
            !cJSON_IsNumber(g) || !cJSON_IsNumber(b)) {

            ESP_LOGE(WS_TAG, "Invalid scheme entry at index %d", i);
            ESP_LOGE(WS_TAG, "end_segment: %s, type: %s, basecolor_enabled: %s, r: %s, g: %s, b: %s",
                     cJSON_Print(end_segment), cJSON_Print(type),
                     cJSON_Print(basecolor_enabled), cJSON_Print(r),
                     cJSON_Print(g), cJSON_Print(b));

            cJSON_Delete(json);
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid scheme entry");
            return ESP_FAIL;
        }
        ESP_LOGI(WS_TAG, "end_segment: %d", end_segment->valueint);
        ESP_LOGI(WS_TAG, "type: %d", end_segment->valueint);
        ESP_LOGI(WS_TAG, "basecolor_enabled: %d", end_segment->valueint);
        ESP_LOGI(WS_TAG, "r: %d", end_segment->valueint);
        ESP_LOGI(WS_TAG, "g: %d", end_segment->valueint);
        ESP_LOGI(WS_TAG, "b: %d", end_segment->valueint);

        digifizCustom.scheme[i].end_segment = end_segment->valueint;
        digifizCustom.scheme[i].type = type->valueint;
        digifizCustom.scheme[i].basecolor_enabled = basecolor_enabled->valueint;
        digifizCustom.scheme[i].r = r->valueint;
        digifizCustom.scheme[i].g = g->valueint;
        digifizCustom.scheme[i].b = b->valueint;
        //Executed only once!
        break;
    }

    cJSON_Delete(json);

    // Store the updated scheme as a blob
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("digifiz", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to open NVS");
        return ESP_FAIL;
    }

    err = nvs_set_blob(nvs_handle, "color_scheme", digifizCustom.scheme, sizeof(digifizCustom.scheme));
    if (err != ESP_OK) {
        nvs_close(nvs_handle);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to save blob");
        return ESP_FAIL;
    }

    err = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);

    if (err != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to commit NVS");
        return ESP_FAIL;
    }

    // Optional: persist to RAM again or recompile if needed
    compileColorScheme();

    httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
    return ESP_OK;
}

static const httpd_uri_t set_color_scheme = {
    .uri      = "/set_color_scheme",
    .method   = HTTP_POST,
    .handler  = set_color_scheme_handler,
    .user_ctx = NULL
};

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
    if (ret)
        goto _ret;
    ret = httpd_register_uri_handler(server, &params_get);
    if (ret)
        goto _ret;
    ret = httpd_register_uri_handler(server, &set_param_post);
    if (ret)
        goto _ret;
    //ret = httpd_register_uri_handler(server, &set_color_scheme);
    //if (ret)
    //    goto _ret;
    ret = httpd_register_uri_handler(server, &oscilloscope_dump);
    if (ret)
        goto _ret;
_ret:
    return ret;
}
