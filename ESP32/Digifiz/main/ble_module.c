#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include <string.h>
#include <inttypes.h>  // <- for PRIu32
#include "gear_estimator.h"

#define GATTS_TAG "NIMBLE_MODULE"
#define SERVICE_UUID  0x00FF
#define CHAR1_UUID    0xFF01  // RPM
#define CHAR2_UUID    0xFF02  // Speed
#define CHAR3_UUID    0xFF03  // Gear

void ble_store_config_init(void);

static uint16_t conn_handle;
static uint16_t char1_handle, char2_handle, char3_handle;
static uint32_t char1_value = 123456, char2_value = 654321; // RPM, Speed
static uint8_t char3_value = 0; // Gear

static const char *device_name = "DigifizBLE";

static int gatt_svr_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                              struct ble_gatt_access_ctxt *ctxt, void *arg) {
    void *val_ptr = NULL;
    size_t val_len = 0;

    if (attr_handle == char1_handle) { val_ptr = &char1_value; val_len = sizeof(char1_value); }
    if (attr_handle == char2_handle) { val_ptr = &char2_value; val_len = sizeof(char2_value); }
    if (attr_handle == char3_handle) { val_ptr = &char3_value; val_len = sizeof(char3_value); }

    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
        if (val_ptr) {
            int rc = os_mbuf_append(ctxt->om, val_ptr, val_len);
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
    } else if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
        if (val_ptr && ctxt->om->om_len == val_len) {
            memcpy(val_ptr, ctxt->om->om_data, val_len);
            return 0;
        }
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    return BLE_ATT_ERR_UNLIKELY;
}

static const struct ble_gatt_svc_def gatt_services[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(SERVICE_UUID),
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = BLE_UUID16_DECLARE(CHAR1_UUID),
                .access_cb = gatt_svr_access_cb,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
                .val_handle = &char1_handle,
            },
            {
                .uuid = BLE_UUID16_DECLARE(CHAR2_UUID),
                .access_cb = gatt_svr_access_cb,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
                .val_handle = &char2_handle,
            },
            {
                .uuid = BLE_UUID16_DECLARE(CHAR3_UUID),
                .access_cb = gatt_svr_access_cb,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
                .val_handle = &char3_handle,
            },
            {0}
        }
    },
    {0}
};

static void ble_app_advertise(void);

static int ble_gap_event_handler(struct ble_gap_event *event, void *arg) {
    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            if (event->connect.status == 0) {
                conn_handle = event->connect.conn_handle;
                ESP_LOGI(GATTS_TAG, "Client connected");
            } else {
                ESP_LOGI(GATTS_TAG, "Connection failed, restarting advertising");
                ble_app_advertise();
            }
            break;

        case BLE_GAP_EVENT_DISCONNECT:
            ESP_LOGI(GATTS_TAG, "Client disconnected");
            ble_app_advertise();
            break;

        case BLE_GAP_EVENT_ADV_COMPLETE:
            ESP_LOGI(GATTS_TAG, "Advertising complete, restarting");
            ble_app_advertise();
            break;

        default:
            break;
    }
    return 0;
}

static void ble_app_advertise(void) {
    struct ble_gap_adv_params adv_params = {
        .conn_mode = BLE_GAP_CONN_MODE_UND,
        .disc_mode = BLE_GAP_DISC_MODE_GEN,
    };

    struct ble_hs_adv_fields fields;
    memset(&fields, 0, sizeof(fields));

    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

    static const ble_uuid16_t adv_uuid16 = BLE_UUID16_INIT(SERVICE_UUID);
    fields.uuids16 = &adv_uuid16;
    fields.num_uuids16 = 1;
    fields.uuids16_is_complete = 1;

    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;

    ble_gap_adv_set_fields(&fields);
    ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER, &adv_params,
                      ble_gap_event_handler, NULL);
}

static void ble_app_on_sync(void) {
    //ble_hs_id_infer_auto(0, NULL);
    ble_app_advertise();
}

void ble_host_task(void *param) {
    nimble_port_run();
    nimble_port_freertos_deinit();
}

void ble_module_init(void) {
    esp_err_t ret;
    printf("[BLE] nimble_port_init...\n");
    ret = nimble_port_init();

    if (ret != ESP_OK) {
        printf("[BLE] Failed to init nimble %d\n", ret);
        return;
    }
    printf("[BLE] nimble_port_init OK\n");

    ble_hs_cfg.sync_cb = ble_app_on_sync;
    printf("[BLE] Set sync_cb\n");

    printf("[BLE] ble_svc_gap_init...\n");
    ble_svc_gap_init();
    printf("[BLE] ble_svc_gap_init OK\n");

    printf("[BLE] ble_svc_gatt_init...\n");
    ble_svc_gatt_init();
    printf("[BLE] ble_svc_gatt_init OK\n");

    printf("[BLE] ble_att_set_preferred_mtu...\n");
    if (ble_att_set_preferred_mtu(512) != 0) 
    {
        printf("[BLE] Failed to set preferred MTU\n");
    }
    else
    {
        printf("[BLE] Preferred MTU set to 512 bytes\n");
    }

    printf("[BLE] ble_gatts_count_cfg...\n");
    ret = ble_gatts_count_cfg(gatt_services);
    if (ret != 0) {
        printf("[BLE] Failed to count GATT services %d\n", ret);
        return;
    }
    printf("[BLE] ble_gatts_count_cfg OK\n");

    printf("[BLE] ble_gatts_add_svcs...\n");
    ret = ble_gatts_add_svcs(gatt_services);
    if (ret != 0) {
        printf("[BLE] Failed to add GATT services %d\n", ret);
        return;
    }
    printf("[BLE] ble_gatts_add_svcs OK\n");

    printf("[BLE] ble_svc_gap_device_name_set...\n");
    ret = ble_svc_gap_device_name_set(device_name);
    if (ret != 0) {
        printf("[BLE] Failed to set device name %d\n", ret);
        return;
    }
    printf("[BLE] Device name set: %s\n", device_name);

    ble_store_config_init();

    printf("[BLE] nimble_port_freertos_init...\n");
    nimble_port_freertos_init(ble_host_task);
    printf("[BLE] nimble_port_freertos_init OK\n");
}


void ble_module_update_values(float rpm, float speed) {
    char1_value = (uint32_t)rpm;
    char2_value = (uint32_t)speed;
    char3_value = (uint8_t)gear_estimator_get_current_gear();
}
