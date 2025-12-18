/**
  ******************************************************************************
  * @file    xparam.h
  * @brief   Parameter helper library shared with ESP32 implementation
  ******************************************************************************
  */
#ifndef INC_XPARAM_H_
#define INC_XPARAM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#define XPARAM_MAGIC 0xABCD1234
#define XPARAM_LOG   1

typedef enum{
        XPARAM_NONE,
        XPARAM_BOOL,
        XPARAM_I8,
        XPARAM_U8,
        XPARAM_I16,
        XPARAM_U16,
        XPARAM_I32,
        XPARAM_U32,
        XPARAM_FLOAT,
        XPARAM_STRING,
}xparam_vtype_t;

#define _COMMON_PARAM_FIELDS() \
                char*                                   p_name;         \
                char*                                   p_info;         \
                xparam_vtype_t          value_type;     \
                float                                   min;            \
                float                                   max;            \
                float                                   step_size;  \
                int                     (*print_cb)(struct xparam_s* param, char* buf);  /*return number of written chars*/ \
                uint8_t                 (*change_cb)(struct xparam_s* param, int16_t steps);  /*return 1 if change was within limits*/ \
                char*                   field_name;                     \

typedef struct xparam_s{
    union {
        uint32_t value;          // parameter value
        uintptr_t _align;        // ensure consistent first-field width
    };
        _COMMON_PARAM_FIELDS()
}xparam_U32_t;

typedef xparam_U32_t xparam_t;

typedef struct{
        union {
                int32_t                         value;
                uintptr_t                       _align;
        };
        _COMMON_PARAM_FIELDS()
}xparam_I32_t;
static_assert(  sizeof(xparam_U32_t) == sizeof(xparam_I32_t),
                "xparams module expects identical size of parameters");

typedef struct{
        union {
                bool                            value;
                uintptr_t                       _align;
        };
        _COMMON_PARAM_FIELDS()
}xparam_BOOL_t;
static_assert(  sizeof(xparam_U32_t) == sizeof(xparam_BOOL_t),
                "xparams module expects identical size of parameters");

typedef struct{
        union {
                int16_t                         value;
                uintptr_t                       _align;
        };
        _COMMON_PARAM_FIELDS()
}xparam_I16_t;
static_assert(  sizeof(xparam_U32_t) == sizeof(xparam_I16_t),
                "xparams module expects identical size of parameters");

typedef struct{
        union {
                uint16_t                        value;
                uintptr_t                       _align;
        };
        _COMMON_PARAM_FIELDS()
}xparam_U16_t;
static_assert(  sizeof(xparam_U32_t) == sizeof(xparam_U16_t),
                "xparams module expects identical size of parameters");

typedef struct{
        union {
                int8_t                          value;
                uintptr_t                       _align;
        };
        _COMMON_PARAM_FIELDS()
}xparam_I8_t;
static_assert(  sizeof(xparam_U32_t) == sizeof(xparam_I8_t),
                "xparams module expects identical size of parameters");

typedef struct{
        union {
                uint8_t                         value;
                uintptr_t                       _align;
        };
        _COMMON_PARAM_FIELDS()
}xparam_U8_t;
static_assert(  sizeof(xparam_U32_t) == sizeof(xparam_U8_t),
                "xparams module expects identical size of parameters");

typedef struct{
        union {
                float                           value;
                uintptr_t                       _align;
        };
        _COMMON_PARAM_FIELDS()
}xparam_FLOAT_t;
static_assert(  sizeof(xparam_U32_t) == sizeof(xparam_FLOAT_t),
                "xparams module expects identical size of parameters");

typedef struct{
        union {
                char*                           value;
                uintptr_t                       _align;
        };
        _COMMON_PARAM_FIELDS()
}xparam_STRING_t;
static_assert(  sizeof(xparam_U32_t) == sizeof(xparam_STRING_t),
                "xparams module expects identical size of parameters");

typedef struct{
    xparam_t* params;
    uint32_t  n_params;
}xparam_table_t;

typedef struct{
        uint32_t data;
        uint32_t key;
        uint32_t crc;
        uint32_t _reserved;
}xparam_store_t;

typedef struct{
        uint32_t                magic;
        uint32_t                n_params;
        uint32_t                _reserved[2];
} xparam_header_t;

typedef struct __attribute__((packed)){
        xparam_header_t                 header;
        xparam_store_t                  params[];
}xparam_img_t;

static_assert(  sizeof(xparam_header_t) == 16,
                "changing xparam_img_header_t size will break already written parameters");
static_assert(  sizeof(xparam_store_t)  == 16,
                "changing xparam_store_t size will break already written parameters");

#define XPARAM_IMAGE_SIZE(n) (sizeof(xparam_img_t) + (n * sizeof(xparam_store_t)))

#define DECLARE_PARAM(_type, _name, ...) xparam_##_type##_t _name;
#define DEFINE_PARAM(_type, _name, ...) \
                ._name = {\
                        .value_type = XPARAM_##_type, \
                        .field_name = #_name #_type, \
                        __VA_ARGS__ \
                },

#define XPARAM_COUNT(T) (sizeof(T) / sizeof(xparam_t))

uint8_t* xparam_table_to_blob(xparam_table_t* table);
uint8_t  xparam_table_from_blob(xparam_table_t* table, uint8_t* buf);
int      xparam_stringify(xparam_t* param, char* buf);
uint8_t  xparam_step_value(xparam_t* param, int16_t n_steps);
uint8_t  xparam_set_value(xparam_t* param, uint32_t value);
size_t   xparam_get_json_buf_size(xparam_table_t* table);
char*    xparam_table_to_json(xparam_table_t* table);
xparam_t* xparam_find_by_name(xparam_table_t* table, const char* name);

#ifdef __cplusplus
}
#endif

#endif
