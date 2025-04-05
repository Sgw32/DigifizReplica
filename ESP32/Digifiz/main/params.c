#include "params.h"
#include "esp_log.h"
#include <string.h>


#define TAG "EEPROM module"
const char LOG_TAG[] = "Digifiz Replica Next";

// digifiz_pars digifiz_parameters;
// digifiz_pars r_d_pars;
uint8_t memory_block_selected = 0;


digifiz_pars_t digifiz_parameters = {
    PARAM_LIST(DEFINE_PARAM)
};

digifiz_stats_t digifiz_status;

xparam_table_t params_table = {
    .params = (xparam_t*)&digifiz_parameters,
    .n_params = XPARAM_COUNT(digifiz_parameters),
};

// bool checkInternalMagicBytes()
// {
//     uint8_t cnt = 0;
//     nvs_handle_t my_handle;
//     esp_err_t err = nvs_open("digifiz", NVS_READWRITE, &my_handle);
//     if (err != ESP_OK) {
//         printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
//     } 
//     else 
//     {
//         for (int j=0;j!=EEPROM_DOUBLING;j++)
//         {
//             for (cnt=0;cnt!=3;cnt++) //What if we have a wrong negative results???
//             {
//                 //Give it 10 chances
//                 size_t size = sizeof(digifiz_pars);
//                 nvs_get_blob(my_handle, memory_blocks[cnt], &r_d_pars, &size);
//                 if ((r_d_pars.header[0]=='D')&&
//                     (r_d_pars.header[1]=='I')&&
//                     (r_d_pars.header[2]=='G')&&
//                     (r_d_pars.header[3]=='I'))
//                 {
//                     memory_block_selected = 0;
//                     return true;
//                 }
//             }
//         }
//     }
//     return false;
// }

void saveParameters()
{
  #ifdef DISABLE_EEPROM
  return;
  #endif
    // Open NVS namespace
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("digifiz", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        // Write parameters data to NVS
        ESP_LOGI(TAG, "writing parameters to NVS...");
        uint8_t* blob_data = xparam_table_to_blob(&params_table);
        if (blob_data){
            nvs_set_blob(my_handle, "store", blob_data, XPARAM_IMAGE_SIZE(params_table.n_params));
            // Commit changes to NVS
            err = nvs_commit(my_handle);
            if (err != ESP_OK) {
                printf("Error (%s) committing data to NVS!\n", esp_err_to_name(err));
            }
            free(blob_data);
            ESP_LOGI(TAG, "write ok");
        }
        else{
            ESP_LOGE(TAG, "Data blob malloc failed during parameters store.");
        }
        
        // Write status data
        ESP_LOGI(TAG, "writing status to NVS...");
        nvs_set_blob(my_handle, "status", (uint8_t*)&digifiz_status, sizeof(digifiz_status));
        // Commit changes to NVS
        err = nvs_commit(my_handle);
        if (err != ESP_OK) {
            printf("Error (%s) committing data to NVS!\n", esp_err_to_name(err));
        }
        else{
            ESP_LOGI(TAG, "write ok");
        }

        // Close NVS
        nvs_close(my_handle);
    }
}

void load_defaults()
{
    ESP_LOGI(TAG, "Restoring default parameter values...");
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("digifiz", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } 
    else
    {
        size_t blob_len = 0;
        err = nvs_get_blob(my_handle, "default", NULL, &blob_len);
        if (err == ESP_OK){
            // found nvs key
            uint8_t* blob = malloc(blob_len);
            if (blob){
                ESP_LOGI(TAG, "Malloc %d bytes.", blob_len);
                nvs_get_blob(my_handle, "default", blob, &blob_len);
                xparam_table_from_blob(&params_table, blob);
                ESP_LOGI(TAG, "Default parameter values loaded.");
                nvs_close(my_handle);
                free(blob);
            }
            else{
                ESP_LOGE(TAG, "Blob malloc failed when reading parameters.");
            }
        }
        else if (err == ESP_ERR_NVS_NOT_FOUND){
            nvs_close(my_handle);
            ESP_LOGW(TAG, "NVS key default not found.");
        }
        else{
            printf("Error (%s) opening NVS key!\n", esp_err_to_name(err));
        }
    }
}


void initEEPROM()
{
    nvs_handle_t my_handle;
    ESP_LOGI(TAG, "initEEPROM started");
    // load_defaults(); //from table, not from memory

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_LOGI(TAG, "EEPROM erased");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);


    #ifdef DISABLE_EEPROM
    return;
    #endif

    err = nvs_open("digifiz", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } 
    else
    {
        // check if default data was stored
        ESP_LOGI(TAG, "Checking for default data");
        size_t default_len = 0;
        err = nvs_get_blob(my_handle, "default", NULL, &default_len);
        if ((err == ESP_ERR_NVS_NOT_FOUND) || (default_len != XPARAM_IMAGE_SIZE(params_table.n_params))){
            // no default data or old data
            ESP_LOGW(TAG, "No default data found. Storing...");
            uint8_t* blob_data = xparam_table_to_blob(&params_table);
            if (blob_data){
                nvs_set_blob(my_handle, "default", blob_data, XPARAM_IMAGE_SIZE(params_table.n_params));
                // Commit changes to NVS
                err = nvs_commit(my_handle);
                if (err != ESP_OK) {
                    printf("Error (%s) committing data to NVS!\n", esp_err_to_name(err));
                }
                free(blob_data);
                ESP_LOGI(TAG, "write ok");
            }
            else{
                ESP_LOGE(TAG, "Data blob malloc failed during parameters store.");
            }
        }

        // load parameters
        // first get length of stored data
        size_t blob_len = 0;
        err = nvs_get_blob(my_handle, "store", NULL, &blob_len);
        if (err == ESP_OK){
            // found nvs key
            uint8_t* blob = malloc(blob_len);
            if (blob){
                ESP_LOGI(TAG, "Malloc %d bytes.", blob_len);
                nvs_get_blob(my_handle, "store", blob, &blob_len);
                if (xparam_table_from_blob(&params_table, blob)){
                    ESP_LOGI(TAG, "Parameters load done.");
                }
                else{
                    ESP_LOGW(TAG, "No parameters image found. Loading default.");
                    saveParameters();
                }
                free(blob);
            }
            else{
                ESP_LOGE(TAG, "Blob malloc failed when reading parameters.");
            }
        }
        else if (err == ESP_ERR_NVS_NOT_FOUND){
            ESP_LOGW(TAG, "NVS key not found. Loading default.");
            saveParameters();
        }
        else{
            printf("Error (%s) opening NVS key!\n", esp_err_to_name(err));
        }
        
        
        // load status 
        size_t status_len = 0;
        err = nvs_get_blob(my_handle, "status", NULL, &status_len);
        if (err == ESP_OK){
            if (status_len == sizeof(digifiz_status)){
                nvs_get_blob(my_handle, "status", (uint8_t*)&digifiz_status, &status_len);
                ESP_LOGI(TAG, "Status load done.");
            }
            else{
                ESP_LOGE(TAG, "Digifiz status size changed. Not loaded.");
            }
        }
        else{
            ESP_LOGE(TAG, "Digifiz status load failed.");
        }
    }
    nvs_close(my_handle);

    ESP_LOGI(TAG, "initEEPROM ended");
}
