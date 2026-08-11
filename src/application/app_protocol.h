#ifndef APP_PROTOCOL_H
#define APP_PROTOCOL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define APP_PROTOCOL_REQUEST_SIZE 5u
#define APP_PROTOCOL_MAX_RESPONSE_SIZE 66u
#define APP_PROTOCOL_DEFAULT_RESPONSE_DELAY_MS 50u

#define APP_PROTOCOL_HEADER_FA 0xFAu
#define APP_PROTOCOL_HEADER_DA 0xDAu
#define APP_PROTOCOL_OPERATION_READ 0x52u
#define APP_PROTOCOL_OPERATION_WRITE 0x57u

typedef enum {
    APP_EFFECT_NONE = 0,
    APP_EFFECT_TOUCH_0,
    APP_EFFECT_TOUCH_1,
    APP_EFFECT_SET_CHAMBER_STATE,
} app_effect_type_t;

typedef struct {
    app_effect_type_t type;
    uint8_t value;
} app_effect_t;

typedef struct {
    uint8_t chamber_status;
} app_protocol_inputs_t;

typedef struct {
    uint8_t data[APP_PROTOCOL_MAX_RESPONSE_SIZE];
    size_t length;
} app_response_t;

typedef struct {
    bool has_response;
    app_response_t response;
    app_effect_t effect;
} app_protocol_result_t;

void app_protocol_handle(
    const uint8_t request[APP_PROTOCOL_REQUEST_SIZE],
    const app_protocol_inputs_t *inputs,
    app_protocol_result_t *result);

#ifdef __cplusplus
}
#endif

#endif

