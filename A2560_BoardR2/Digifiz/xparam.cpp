#include "xparam.h"
#include <string.h>
#include <stdio.h>

#if XPARAM_LOG == 1
    #define LOG(...) printf(__VA_ARGS__)
#else
    #define LOG(...)
#endif

static uint32_t xparam_crc32(uint8_t *buf, size_t size)
{
   int j;
   uint32_t byte, crc, mask;

   crc = 0xFFFFFFFF;
   for (int i = 0; i < (int)size; i++){
      byte = buf[i];
      crc = crc ^ byte;
      for (j = 7; j >= 0; j--) {
         mask = -(crc & 1);
         crc = (crc >> 1) ^ (0xEDB88320 & mask);
      }
   }
   return ~crc;
}

static uint32_t xparam_get_key(xparam_t* param){
    return xparam_crc32((uint8_t*)param->field_name, strlen(param->field_name));
}

static uint32_t xparam_get_value_crc(uint32_t data){
    return xparam_crc32((uint8_t*)&data, sizeof(uint32_t));
}

int xparam_value_type_str(xparam_vtype_t vtype, char* buf){
        switch (vtype) {
                case XPARAM_NONE:
                        return 0;
                case XPARAM_U8:
                        return sprintf(buf, "uint8_t");
                case XPARAM_BOOL:
                        return sprintf(buf, "bool");
                case XPARAM_I8:
                        return sprintf(buf, "int8_t");
                case XPARAM_U16:
                        return sprintf(buf, "uint16_t");
                case XPARAM_I16:
                        return sprintf(buf, "int16_t");
                case XPARAM_U32:
                        return sprintf(buf, "uint32_t");
                case XPARAM_I32:
                        return sprintf(buf, "int32_t");
                case XPARAM_FLOAT:
                        return sprintf(buf, "float");
                case XPARAM_STRING:
                        return sprintf(buf, "str");
        }
        return 0;
}

uint8_t* xparam_table_to_blob(xparam_table_t* table)
{
    xparam_img_t* img = (xparam_img_t*)malloc(XPARAM_IMAGE_SIZE(table->n_params));
    if (img == NULL){
        return NULL;
    }
    img->header.magic = XPARAM_MAGIC;
    img->header.n_params = table->n_params;
    for (uint32_t i = 0; i < table->n_params; i++){
        xparam_t* param = &table->params[i];
        img->params[i].data = param->value;
        img->params[i].key = xparam_get_key(param);
        img->params[i].crc = xparam_get_value_crc(param->value);
    }
    return (uint8_t*) img;
}

uint8_t xparam_table_from_blob(xparam_table_t* table, uint8_t* buf)
{
    xparam_img_t* img = (xparam_img_t*)buf;
        if (img->header.magic == XPARAM_MAGIC){
                for (uint32_t i = 0; i < table->n_params; i++){
                        xparam_t* param = &table->params[i];
            if (param->value_type == XPARAM_STRING){
                continue;
            }
                        uint32_t key_calc = xparam_get_key(param);
            uint8_t found = 0;
                        for (uint32_t j = 0; j < img->header.n_params; j++){
                                if (img->params[j].key == key_calc){
                    uint32_t crc_calc = xparam_get_value_crc(img->params[j].data);
                    if (crc_calc == img->params[j].crc){
                        param->value = img->params[j].data;
                        xparam_step_value(param, 0);
                    }
                    else{
                        LOG("XPARAM load table: Value data corrupted for parameter #%u %s. Default value kept.\n", i, param->p_name);
                    }
                    found = 1;
                                }
                        }
                        if(!found){
                LOG("XPARAM load table: Missing data for parameter #%u %s. Default value kept.\n", i, param->p_name);
            }
                }
                return 1;
        }
    LOG("XPARAM load table: No magic in blob.\n");
        return 0;
}

#define PRINTF_FLOAT_SUPPORT
int xparam_stringify(xparam_t* param, char* buf)
{
        if (param->print_cb){
                return param->print_cb(param, buf);
        }
        if (XPARAM_U32 == param->value_type){
                return sprintf(buf,"%lu" , (unsigned long) param->value);
        }
        else if (XPARAM_I32 == param->value_type){
        xparam_I32_t* p_param = (xparam_I32_t*)param;
                return sprintf(buf,"%li" , (long) p_param->value);
        }
        else if (XPARAM_BOOL == param->value_type){
                xparam_BOOL_t* p_param = (xparam_BOOL_t*)param;
                if (p_param->value){
                        return sprintf(buf,"true");
                }
                else{
                        return sprintf(buf,"false");
                }
        }
        else if (XPARAM_U16 == param->value_type){
        xparam_U16_t* p_param = (xparam_U16_t*)param;
                return sprintf(buf,"%u" , p_param->value);
        }
        else if (XPARAM_I16 == param->value_type){
        xparam_I16_t* p_param = (xparam_I16_t*)param;
                return sprintf(buf,"%i" , p_param->value);
        }
        else if (XPARAM_U8 == param->value_type){
        xparam_U8_t* p_param = (xparam_U8_t*)param;
                return sprintf(buf,"%u" , p_param->value);
        }
        else if (XPARAM_I8 == param->value_type){
        xparam_I8_t* p_param = (xparam_I8_t*)param;
                return sprintf(buf,"%i" , p_param->value);
        }
        else if (XPARAM_FLOAT == param->value_type){
        xparam_FLOAT_t* p_param = (xparam_FLOAT_t*)param;
                return sprintf(buf,"%0.2f" , (double)p_param->value);
        }
        else if (XPARAM_STRING == param->value_type){
        xparam_STRING_t* p_param = (xparam_STRING_t*)param;
                return sprintf(buf,"%s" , p_param->value);
        }
        else{
                return sprintf(buf,"NA");
        }
}

uint8_t xparam_step_value(xparam_t* param, int16_t n_steps)
{
        if (param->change_cb){
                return param->change_cb(param, n_steps);
        }
        if (XPARAM_BOOL == param->value_type){
        xparam_BOOL_t* p_param = (xparam_BOOL_t*)param;
        if ((p_param->value == false) && (n_steps > 0)){
            p_param->value = true;
            return 1;
        }
        else if ((p_param->value == true) && (n_steps < 0)){
            p_param->value = false;
            return 1;
        }
        return 0;
        }
        else if (XPARAM_I32 == param->value_type){
        xparam_I32_t* p_param = (xparam_I32_t*)param;
        if (((float)p_param->value + n_steps * param->step_size) >= param->min){
            if (((float)p_param->value + n_steps * param->step_size) <= param->max){
                p_param->value += n_steps * param->step_size;
                return 1;
            }
        }
        return 0;
        }
        else if (XPARAM_U32 == param->value_type){
        xparam_U32_t* p_param = (xparam_U32_t*)param;
        if (((float)p_param->value + n_steps * param->step_size) >= param->min){
            if (((float)p_param->value + n_steps * param->step_size) <= param->max){
                p_param->value += n_steps * param->step_size;
                return 1;
            }
        }
        return 0;
        }
        else if (XPARAM_I16 == param->value_type){
        xparam_I16_t* p_param = (xparam_I16_t*)param;
        if (((float)p_param->value + n_steps * param->step_size) >= param->min){
            if (((float)p_param->value + n_steps * param->step_size) <= param->max){
                p_param->value += n_steps * param->step_size;
                return 1;
            }
        }
        return 0;
        }
        else if (XPARAM_U16 == param->value_type){
        xparam_U16_t* p_param = (xparam_U16_t*)param;
        if (((float)p_param->value + n_steps * param->step_size) >= param->min){
            if (((float)p_param->value + n_steps * param->step_size) <= param->max){
                p_param->value += n_steps * param->step_size;
                return 1;
            }
        }
        return 0;
        }
        else if (XPARAM_I8 == param->value_type){
        xparam_I8_t* p_param = (xparam_I8_t*)param;
        if (((float)p_param->value + n_steps * param->step_size) >= param->min){
            if (((float)p_param->value + n_steps * param->step_size) <= param->max){
                p_param->value += n_steps * param->step_size;
                return 1;
            }
        }
        return 0;
        }
        else if (XPARAM_U8 == param->value_type){
        xparam_U8_t* p_param = (xparam_U8_t*)param;
        if (((float)p_param->value + n_steps * param->step_size) >= param->min){
            if (((float)p_param->value + n_steps * param->step_size) <= param->max){
                p_param->value += n_steps * param->step_size;
                return 1;
            }
        }
        return 0;
        }
        else if (XPARAM_FLOAT == param->value_type){
        xparam_FLOAT_t* p_param = (xparam_FLOAT_t*)param;
        if (((float)p_param->value + n_steps * param->step_size) >= param->min){
            if (((float)p_param->value + n_steps * param->step_size) <= param->max){
                p_param->value += n_steps * param->step_size;
                return 1;
            }
        }
        return 0;
        }
        return 0;
}

uint8_t xparam_set_value(xparam_t* param, uint32_t value)
{
        if (XPARAM_BOOL == param->value_type){
        xparam_BOOL_t* p_param = (xparam_BOOL_t*)param;
        p_param->value = value;
        return 1;
        }
        else if (XPARAM_I32 == param->value_type){
        xparam_I32_t* p_param = (xparam_I32_t*)param;
        p_param->value = value;
        xparam_step_value(param, 0);
        return 1;
        }
        else if (XPARAM_U32 == param->value_type){
        param->value = value;
        xparam_step_value(param, 0);
        return 1;
        }
        else if (XPARAM_I16 == param->value_type){
        xparam_I16_t* p_param = (xparam_I16_t*)param;
        p_param->value = value;
        xparam_step_value(param, 0);
        return 1;
        }
        else if (XPARAM_U16 == param->value_type){
        xparam_U16_t* p_param = (xparam_U16_t*)param;
        p_param->value = value;
        xparam_step_value(param, 0);
        return 1;
        }
        else if (XPARAM_I8 == param->value_type){
        xparam_I8_t* p_param = (xparam_I8_t*)param;
        p_param->value = value;
        xparam_step_value(param, 0);
        return 1;
        }
        else if (XPARAM_U8 == param->value_type){
        xparam_U8_t* p_param = (xparam_U8_t*)param;
        p_param->value = value;
        xparam_step_value(param, 0);
        return 1;
        }
        else if (XPARAM_FLOAT == param->value_type){
        xparam_FLOAT_t* p_param = (xparam_FLOAT_t*)param;
        memcpy(&p_param->value, &value, sizeof(float));
        xparam_step_value(param, 0);
        return 1;
        }
        return 0;
}

size_t xparam_get_json_buf_size(xparam_table_t* table){
        size_t size = 2;
        char val_buf[64];
        for (uint32_t i = 0; i < table->n_params; i++){
            xparam_t* param = &table->params[i];
            if (param->value_type == XPARAM_STRING){
                continue;
            }
            size += xparam_stringify(param, val_buf);
            size += xparam_value_type_str(param->value_type, val_buf);
            size += strlen(param->p_name);
            size += strlen(param->p_info);
            size += 128;
        }
        return size;
}

char* xparam_to_json(xparam_t* param, char* buf){
        char str_buf[64];
        buf += sprintf(buf, "{\"name\":\"%s\",", param->p_name);
        buf += sprintf(buf, "\"info\":\"%s\",", param->p_info);
        buf += sprintf(buf, "\"value\":");
        xparam_stringify(param, str_buf);
        buf += sprintf(buf, "\"%s\",", str_buf);
        buf += sprintf(buf, "\"type\":");
        xparam_value_type_str(param->value_type, str_buf);
        buf += sprintf(buf, "\"%s\",", str_buf);
        buf += sprintf(buf, "\"id\":\"%08lx\"}", (unsigned long) xparam_get_key(param));
        return buf;
}

char* xparam_table_to_json(xparam_table_t* table){
        char* ret = (char*)malloc(xparam_get_json_buf_size(table));
        if (ret == NULL){
                LOG("xparam_table_to_json: buffer malloc failed!\n");
                return NULL;
        }
        char* buf = ret;
        buf += sprintf(buf, "[");
        for (uint32_t i = 0; i < table->n_params; i++){
                buf = xparam_to_json(&table->params[i], buf);
                if (i < table->n_params - 1){
                        buf += sprintf(buf, ",");
                }
        }
        buf += sprintf(buf, "]");
        return ret;
}

xparam_t* xparam_find_by_name(xparam_table_t* table, const char* name) {
    for (uint32_t i = 0; i < table->n_params; i++) {
        xparam_t* param = &table->params[i];
        if (strcmp(param->field_name, name) == 0) {
            return param;
        }
    }
    return NULL;
}
