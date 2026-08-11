#include "app_protocol.h"

#include <string.h>

#define DEVICE_3 0x03u

static const uint8_t RESPONSE_ACK[] = {0xC3, 0x0D, 0x0A};
static const uint8_t RESPONSE_IDENTIFY[] = {0xFA, 0x01, 0x0D, 0x0A};
static const uint8_t RESPONSE_CONTENT[] = {
    0xFA, 0x00, 0x08, 0x01, 0x02, 0x00, 0x02, 0x01, 0x02, 0x0D, 0x0A,
};
static const uint8_t RESPONSE_KNOB[] = {0xFA, 0x01, 0x03, 0x05, 0x0D, 0x0A};
static const uint8_t RESPONSE_PART_NUMBER[] = {
    0xFA, 0x01, 0x0A, 0x46, 0x49, 0x44, 0x4D, 0x53, 0x57, 0x23, 0x30, 0x0D, 0x0A,
};
static const uint8_t RESPONSE_FIDM_SW[] = {
    0xFA, 0x01, 0x0A, 0x46, 0x49, 0x44, 0x4D, 0x53, 0x57, 0x23, 0x31, 0x0D, 0x0A,
};
static const uint8_t RESPONSE_FIDM_HW[] = {
    0xFA, 0x01, 0x0A, 0x46, 0x49, 0x44, 0x4D, 0x48, 0x57, 0x23, 0x31, 0x0D, 0x0A,
};
static const uint8_t RESPONSE_FIDM_BL[] = {
    0xFA, 0x01, 0x0A, 0x46, 0x49, 0x44, 0x4D, 0x42, 0x4C, 0x23, 0x31, 0x0D, 0x0A,
};
static const uint8_t RESPONSE_TCON[] = {
    0xFA, 0x01, 0x0A, 0x54, 0x43, 0x4F, 0x4E, 0x58, 0x58, 0x23, 0x31, 0x0D, 0x0A,
};
static const uint8_t RESPONSE_TOUCH_VERSION[] = {
    0xFA, 0x01, 0x0A, 0x54, 0x4F, 0x55, 0x43, 0x48, 0x58, 0x23, 0x31, 0x0D, 0x0A,
};
static const uint8_t RESPONSE_LCM_VERSION[] = {
    0xFA, 0x01, 0x0A, 0x4C, 0x43, 0x4D, 0x58, 0x58, 0x58, 0x23, 0x31, 0x0D, 0x0A,
};
static const uint8_t RESPONSE_BRIGHTNESS[] = {0xFA, 0x01, 0x03, 0x64, 0x0D, 0x0A};
static const uint8_t RESPONSE_DIAGNOSTIC[] = {
    0xFA, 0x01, 0x06, 0x00, 0x02, 0x03, 0x04, 0x0D, 0x0A,
};
static const uint8_t RESPONSE_VOLTAGE[] = {0xFA, 0x01, 0x04, 0x00, 0x8A, 0x0D, 0x0A};
static const uint8_t RESPONSE_FIDM_TEMP[] = {0xFA, 0x01, 0x05, 0x4A, 0x4B, 0x4C, 0x0D, 0x0A};
static const uint8_t RESPONSE_CHAMBER_TEMP[] = {0xFA, 0x01, 0x03, 0x4B, 0x0D, 0x0A};
static const uint8_t RESPONSE_ADC[] = {0xFA, 0x01, 0x04, 0x01, 0x12, 0x0D, 0x0A};
static const uint8_t RESPONSE_CURRENT[] = {0xFA, 0x01, 0x04, 0x01, 0x23, 0x0D, 0x0A};
static const uint8_t RESPONSE_TOUCH_DATA[] = {
    0xFA, 0x01, 0x3F, 0x0A, 0x00, 0x11, 0x01, 0x12, 0x00, 0xF2,
    0x00, 0x21, 0x02, 0x12, 0x01, 0x11, 0x00, 0x31, 0x03, 0x12,
    0x01, 0xA0, 0x00, 0x41, 0x04, 0x12, 0x02, 0x00, 0x00, 0x51,
    0x05, 0x12, 0x02, 0x14, 0x00, 0x61, 0x06, 0x12, 0x02, 0x58,
    0x00, 0x71, 0x03, 0xF4, 0x02, 0x65, 0x00, 0x81, 0x08, 0x00,
    0x02, 0x74, 0x00, 0x91, 0x02, 0xF4, 0x02, 0x84, 0x00, 0xA1,
    0x01, 0xF4, 0x03, 0x84, 0x0D, 0x0A,
};
static const uint8_t RESPONSE_ALS[] = {0xFA, 0x01, 0x04, 0x01, 0x05, 0x0D, 0x0A};
static const uint8_t RESPONSE_CCT[] = {
    0xFA, 0x01, 0x08, 0x01, 0x02, 0x00, 0x02, 0x01, 0x02, 0x0D, 0x0A,
};
static const uint8_t RESPONSE_DA_STATUS[] = {
    0xDA, 0x00, 0x08, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x0D, 0x0A,
};
static const uint8_t RESPONSE_DA_02_STATUS[] = {0xDA, 0x02, 0x05, 0x01, 0x01, 0x01, 0x0D, 0x0A};
static const uint8_t RESPONSE_DA_03_STATUS[] = {
    0xDA, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0D, 0x0A,
};
static const uint8_t RESPONSE_DA_CURRENT[] = {0xDA, 0x06, 0x06, 0x01, 0x01, 0x01, 0x01, 0x0D, 0x0A};
static const uint8_t RESPONSE_DA_VOLTAGE_1[] = {0xDA, 0x05, 0x04, 0x01, 0x01, 0x0D, 0x0A};
static const uint8_t RESPONSE_DA_VOLTAGE_2[] = {0xDA, 0x07, 0x04, 0x01, 0x01, 0x0D, 0x0A};
static const uint8_t RESPONSE_DA_CHAMBER[] = {0xDA, 0x20, 0x03, 0x01, 0x0D, 0x0A};

_Static_assert(sizeof(RESPONSE_TOUCH_DATA) == APP_PROTOCOL_MAX_RESPONSE_SIZE,
    "maximum response size must include touch data");

static void set_response(
    app_protocol_result_t *result,
    const uint8_t *data,
    size_t length)
{
    if (length > sizeof(result->response.data)) {
        return;
    }

    memcpy(result->response.data, data, length);
    result->response.length = length;
    result->has_response = true;
}

static void set_device(app_protocol_result_t *result, uint8_t device)
{
    result->response.data[1] = device;
}

static const uint8_t *information_response(uint8_t parameter)
{
    static const uint8_t *const responses[] = {
        RESPONSE_PART_NUMBER,
        RESPONSE_FIDM_HW,
        RESPONSE_FIDM_SW,
        RESPONSE_FIDM_BL,
        RESPONSE_TCON,
        RESPONSE_TOUCH_VERSION,
        RESPONSE_LCM_VERSION,
    };

    return parameter < (sizeof(responses) / sizeof(responses[0]))
        ? responses[parameter]
        : NULL;
}

static void handle_fa_read(
    uint8_t device,
    uint8_t category,
    uint8_t parameter,
    app_protocol_result_t *result)
{
    const uint8_t *response;

    switch (category) {
        case 0x1A:
            set_response(result, RESPONSE_IDENTIFY, sizeof(RESPONSE_IDENTIFY));
            break;
        case 0x1B:
            set_response(result, RESPONSE_CONTENT, sizeof(RESPONSE_CONTENT));
            break;
        case 0x1C:
            set_response(result, RESPONSE_KNOB, sizeof(RESPONSE_KNOB));
            set_device(result, device);
            break;
        case 0x00:
            response = information_response(parameter);
            if (response != NULL) {
                set_response(result, response, sizeof(RESPONSE_PART_NUMBER));
                set_device(result, device);
                result->response.data[10] = (uint8_t)(0x30u + parameter);
            }
            break;
        case 0x01:
            set_response(result, RESPONSE_BRIGHTNESS, sizeof(RESPONSE_BRIGHTNESS));
            set_device(result, device);
            break;
        case 0x04:
            switch (parameter) {
                case 0x01:
                    set_response(result, RESPONSE_DIAGNOSTIC, sizeof(RESPONSE_DIAGNOSTIC));
                    break;
                case 0x02:
                    set_response(result, RESPONSE_VOLTAGE, sizeof(RESPONSE_VOLTAGE));
                    break;
                case 0x03:
                    set_response(result, RESPONSE_FIDM_TEMP, sizeof(RESPONSE_FIDM_TEMP));
                    break;
                case 0x04:
                    set_response(result, RESPONSE_CHAMBER_TEMP, sizeof(RESPONSE_CHAMBER_TEMP));
                    break;
                case 0x05:
                    set_response(result, RESPONSE_ADC, sizeof(RESPONSE_ADC));
                    break;
                default:
                    break;
            }
            if (result->has_response) {
                set_device(result, device);
            }
            break;
        case 0x05:
            set_response(result, RESPONSE_CURRENT, sizeof(RESPONSE_CURRENT));
            set_device(result, device);
            break;
        case 0x06:
            set_response(result, RESPONSE_TOUCH_DATA, sizeof(RESPONSE_TOUCH_DATA));
            set_device(result, device);
            break;
        case 0x0A:
            if (parameter == 0x00u) {
                set_response(result, RESPONSE_ALS, sizeof(RESPONSE_ALS));
            } else if (parameter == 0x01u) {
                set_response(result, RESPONSE_CCT, sizeof(RESPONSE_CCT));
            } else {
                set_response(result, RESPONSE_ACK, sizeof(RESPONSE_ACK));
            }
            break;
        default:
            break;
    }
}

static bool is_generic_da_status_device(uint8_t device)
{
    switch (device) {
        case 0x01:
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
        case 0x09:
            return true;
        default:
            return false;
    }
}

static void handle_da_read(
    uint8_t device,
    uint8_t category,
    uint8_t parameter,
    uint8_t chamber_status,
    app_protocol_result_t *result)
{
    if (device == 0x02u) {
        set_response(result, RESPONSE_DA_02_STATUS, sizeof(RESPONSE_DA_02_STATUS));
    } else if (device == 0x03u) {
        set_response(result, RESPONSE_DA_03_STATUS, sizeof(RESPONSE_DA_03_STATUS));
        set_device(result, device);
    } else if (is_generic_da_status_device(device)) {
        set_response(result, RESPONSE_DA_STATUS, sizeof(RESPONSE_DA_STATUS));
        set_device(result, device);
    } else if (device == 0x0Cu) {
        set_response(result, RESPONSE_DA_CURRENT, sizeof(RESPONSE_DA_CURRENT));
        result->response.data[3] = parameter;
        result->response.data[4] = parameter;
        result->response.data[5] = parameter;
        result->response.data[6] = parameter;
    } else if (device == 0x0Du) {
        if (category == 0x01u) {
            set_response(result, RESPONSE_DA_VOLTAGE_1, sizeof(RESPONSE_DA_VOLTAGE_1));
            result->response.data[3] = parameter;
            result->response.data[4] = parameter;
        } else {
            const uint8_t incremented = (uint8_t)(parameter + 1u);
            set_response(result, RESPONSE_DA_VOLTAGE_2, sizeof(RESPONSE_DA_VOLTAGE_2));
            result->response.data[3] = incremented;
            result->response.data[4] = incremented;
        }
    } else if (device == 0x20u) {
        set_response(result, RESPONSE_DA_CHAMBER, sizeof(RESPONSE_DA_CHAMBER));
        result->response.data[3] = chamber_status;
    } else {
        set_response(result, RESPONSE_ACK, sizeof(RESPONSE_ACK));
    }
}

static void handle_write(
    uint8_t header,
    uint8_t device,
    uint8_t category,
    uint8_t parameter,
    app_protocol_result_t *result)
{
    if (header != APP_PROTOCOL_HEADER_DA) {
        set_response(result, RESPONSE_ACK, sizeof(RESPONSE_ACK));
        return;
    }

    if (device != DEVICE_3) {
        return;
    }

    if (category == 0x01u) {
        result->effect.type = APP_EFFECT_TOUCH_0;
        result->effect.value = parameter;
    } else if (category == 0x02u) {
        result->effect.type = APP_EFFECT_TOUCH_1;
        result->effect.value = parameter;
    } else if (category == 0x0Au) {
        result->effect.type = APP_EFFECT_SET_CHAMBER_STATE;
        result->effect.value = parameter;
    }

    set_response(result, RESPONSE_ACK, sizeof(RESPONSE_ACK));
}

void app_protocol_handle(
    const uint8_t request[APP_PROTOCOL_REQUEST_SIZE],
    const app_protocol_inputs_t *inputs,
    app_protocol_result_t *result)
{
    const uint8_t chamber_status = inputs == NULL ? 0u : inputs->chamber_status;

    if (request == NULL || result == NULL) {
        return;
    }

    memset(result, 0, sizeof(*result));

    if (request[1] == APP_PROTOCOL_OPERATION_READ) {
        if (request[0] == APP_PROTOCOL_HEADER_FA) {
            handle_fa_read(request[2], request[3], request[4], result);
        } else if (request[0] == APP_PROTOCOL_HEADER_DA) {
            handle_da_read(
                request[2], request[3], request[4], chamber_status, result);
        }
    } else if (request[1] == APP_PROTOCOL_OPERATION_WRITE) {
        handle_write(request[0], request[2], request[3], request[4], result);
    }
}

