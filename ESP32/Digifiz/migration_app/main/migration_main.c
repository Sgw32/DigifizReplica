#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "esp_err.h"
#include "esp_check.h"
#include "esp_flash.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_wifi_connect.h"

#define NEW_OTA0_OFFSET       0x010000U
#define PARTITION_TABLE_ADDR  0x008000U
#define PARTITION_TABLE_SIZE  0x001000U
#define OTADATA_ADDR          0x00d000U
#define OTADATA_SIZE          0x002000U
#define COPY_BLOCK_SIZE       0x001000U

extern const uint8_t partition_table_start[] asm("_binary_partition_table_bin_start");
extern const uint8_t partition_table_end[] asm("_binary_partition_table_bin_end");
extern const uint8_t migration_html_start[] asm("_binary_migration_html_start");
extern const uint8_t migration_html_end[] asm("_binary_migration_html_end");

static const char *TAG = "partition_migration";
static bool migration_complete;

static bool ranges_overlap(uint32_t first_address, size_t first_length,
                           uint32_t second_address, size_t second_length)
{
    return first_address < second_address + second_length &&
           second_address < first_address + first_length;
}

static esp_err_t copy_flash(uint32_t source, uint32_t destination, size_t length)
{
    uint8_t buffer[COPY_BLOCK_SIZE];
    ESP_RETURN_ON_ERROR(esp_flash_erase_region(esp_flash_default_chip, destination, length), TAG,
                        "erase destination");
    for (size_t offset = 0; offset < length; offset += sizeof(buffer)) {
        ESP_RETURN_ON_ERROR(esp_flash_read(esp_flash_default_chip, buffer, source + offset,
                                          sizeof(buffer)), TAG, "read source");
        ESP_RETURN_ON_ERROR(esp_flash_write(esp_flash_default_chip, buffer, destination + offset,
                                           sizeof(buffer)), TAG, "write destination");
    }
    return ESP_OK;
}

static esp_err_t verify_flash(uint32_t source, uint32_t destination, size_t length)
{
    uint8_t source_data[COPY_BLOCK_SIZE];
    uint8_t destination_data[COPY_BLOCK_SIZE];
    for (size_t offset = 0; offset < length; offset += sizeof(source_data)) {
        ESP_RETURN_ON_ERROR(esp_flash_read(esp_flash_default_chip, source_data, source + offset,
                                          sizeof(source_data)), TAG, "verify source read");
        ESP_RETURN_ON_ERROR(esp_flash_read(esp_flash_default_chip, destination_data,
                                          destination + offset, sizeof(destination_data)), TAG,
                            "verify destination read");
        if (memcmp(source_data, destination_data, sizeof(source_data)) != 0) {
            ESP_LOGE(TAG, "verification failed at offset 0x%x", (unsigned)offset);
            return ESP_ERR_INVALID_CRC;
        }
    }
    return ESP_OK;
}

/* Some historical 2 MB-slot layouts require one hop to old ota_1 first. */
static esp_err_t relocate_to_safe_slot(const esp_partition_t *running)
{
    const esp_partition_t *safe = esp_partition_find_first(ESP_PARTITION_TYPE_APP,
                                                            ESP_PARTITION_SUBTYPE_APP_OTA_1,
                                                            "ota_1");
    if (safe == NULL || safe->size < running->size || safe->address == running->address ||
        ranges_overlap(safe->address, safe->size, NEW_OTA0_OFFSET, running->size)) {
        return ESP_ERR_NOT_FOUND;
    }
    ESP_LOGW(TAG, "relocating migration image from 0x%lx to safe slot 0x%lx",
             (unsigned long)running->address, (unsigned long)safe->address);
    ESP_RETURN_ON_ERROR(copy_flash(running->address, safe->address, running->size), TAG,
                        "copy to safe slot");
    ESP_RETURN_ON_ERROR(verify_flash(running->address, safe->address, running->size), TAG,
                        "verify safe slot");
    ESP_RETURN_ON_ERROR(esp_ota_set_boot_partition(safe), TAG, "select safe slot");
    esp_restart();
    return ESP_OK;
}

static esp_err_t install_new_table(const esp_partition_t *running)
{
    const size_t table_length = partition_table_end - partition_table_start;
    if (ranges_overlap(running->address, running->size, NEW_OTA0_OFFSET, running->size) ||
        table_length > PARTITION_TABLE_SIZE) {
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGW(TAG, "copying migration image to new ota_0");
    ESP_RETURN_ON_ERROR(copy_flash(running->address, NEW_OTA0_OFFSET, running->size), TAG,
                        "copy new ota_0");
    ESP_RETURN_ON_ERROR(verify_flash(running->address, NEW_OTA0_OFFSET, running->size), TAG,
                        "verify new ota_0");

    /* Clear the old OTA selection so the bootloader scans and finds new ota_0. */
    ESP_RETURN_ON_ERROR(esp_flash_erase_region(esp_flash_default_chip, OTADATA_ADDR, OTADATA_SIZE),
                        TAG, "erase otadata");
    /* This is intentionally last: until this succeeds the old layout remains bootable. */
    ESP_RETURN_ON_ERROR(esp_flash_erase_region(esp_flash_default_chip, PARTITION_TABLE_ADDR,
                                               PARTITION_TABLE_SIZE), TAG, "erase old table");
    ESP_RETURN_ON_ERROR(esp_flash_write(esp_flash_default_chip, partition_table_start,
                                       PARTITION_TABLE_ADDR, table_length), TAG, "write new table");
    ESP_LOGW(TAG, "partition table installed; restarting");
    esp_restart();
    return ESP_OK;
}

static esp_err_t root_handler(httpd_req_t *request)
{
    httpd_resp_set_type(request, "text/html");
    if (!migration_complete) {
        return httpd_resp_sendstr(request,
                                  "<!doctype html><h1>Migration not complete</h1>"
                                  "<p>Do not power off the device.</p>");
    }
    return httpd_resp_send(request, (const char *)migration_html_start,
                           migration_html_end - migration_html_start);
}

static esp_err_t update_handler(httpd_req_t *request)
{
    if (!migration_complete || request->content_len <= 0) {
        return httpd_resp_send_err(request, HTTPD_400_BAD_REQUEST, "Migration incomplete or empty file");
    }
    const esp_partition_t *target = esp_ota_get_next_update_partition(NULL);
    esp_ota_handle_t handle;
    esp_err_t error = esp_ota_begin(target, request->content_len, &handle);
    uint8_t buffer[4096];
    int remaining = request->content_len;
    while (error == ESP_OK && remaining > 0) {
        int received = httpd_req_recv(request, (char *)buffer,
                                      remaining < sizeof(buffer) ? remaining : sizeof(buffer));
        if (received == HTTPD_SOCK_ERR_TIMEOUT) continue;
        if (received <= 0) { error = ESP_FAIL; break; }
        error = esp_ota_write(handle, buffer, received);
        remaining -= received;
    }
    if (error == ESP_OK) error = esp_ota_end(handle); else esp_ota_abort(handle);
    if (error == ESP_OK) error = esp_ota_set_boot_partition(target);
    if (error != ESP_OK) return httpd_resp_send_err(request, HTTPD_500_INTERNAL_SERVER_ERROR,
                                                    esp_err_to_name(error));
    httpd_resp_sendstr(request, "Final firmware installed. Device is restarting.");
    vTaskDelay(pdMS_TO_TICKS(500));
    esp_restart();
    return ESP_OK;
}

static esp_err_t register_handlers(httpd_handle_t server)
{
    const httpd_uri_t root = {.uri = "/", .method = HTTP_GET, .handler = root_handler};
    const httpd_uri_t update = {.uri = "/update", .method = HTTP_POST, .handler = update_handler};
    ESP_RETURN_ON_ERROR(httpd_register_uri_handler(server, &root), TAG, "register root");
    return httpd_register_uri_handler(server, &update);
}

void app_main(void)
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    const esp_partition_t *factory = esp_partition_find_first(ESP_PARTITION_TYPE_APP,
                                                               ESP_PARTITION_SUBTYPE_APP_FACTORY,
                                                               "factory");
    if (factory != NULL) {
        esp_err_t error = ranges_overlap(running->address, running->size,
                                         NEW_OTA0_OFFSET, running->size)
                              ? relocate_to_safe_slot(running)
                              : install_new_table(running);
        ESP_LOGE(TAG, "migration stopped safely: %s", esp_err_to_name(error));
        return;
    }

    migration_complete = true;
    esp_err_t validation = esp_ota_mark_app_valid_cancel_rollback();
    if (validation != ESP_OK && validation != ESP_ERR_NOT_FOUND) {
        ESP_LOGW(TAG, "could not mark migration image valid: %s", esp_err_to_name(validation));
    }
    ESP_LOGI(TAG, "new partition table active; starting final-firmware upload server");
    (void)nvs_wifi_connect();
    ESP_ERROR_CHECK(nvs_wifi_connect_start_http_server(NVS_WIFI_CONNECT_MODE_STAY_ACTIVE,
                                                       register_handlers) == NULL
                        ? ESP_FAIL : ESP_OK);
}
