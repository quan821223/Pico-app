#include "configuration_service.h"

#include <stddef.h>
#include <string.h>

enum {
    CONFIG_STATUS_OK = 0x00u,
    CONFIG_STATUS_INVALID_KEY = 0x01u,
    CONFIG_STATUS_INVALID_VALUE = 0x02u,
    CONFIG_STATUS_STORAGE_ERROR = 0x03u,
};

static bool board_is_allowed(const configuration_service_t *service, board_id_t id)
{
    return (unsigned int)id < 32u &&
        (service->allowed_board_mask & (UINT32_C(1) << (unsigned int)id)) != 0u;
}
static uint16_t value_for_key(
    const configuration_service_t *service,
    uint8_t key,
    bool *valid)
{
    *valid = true;
    switch (key) {
        case CONFIG_KEY_BOARD_PROFILE:
            return (uint16_t)service->current.board_id;
        case CONFIG_KEY_RESPONSE_DELAY_MS:
            return service->current.response_delay_ms;
        case CONFIG_KEY_FRAME_TIMEOUT_MS:
            return service->current.frame_timeout_ms;
        default:
            *valid = false;
            return 0u;
    }
}

static void set_response(
    app_response_t *response,
    uint8_t status,
    uint8_t key,
    uint16_t value)
{
    const uint8_t data[] = {
        CONFIG_PROTOCOL_HEADER,
        status,
        key,
        (uint8_t)(value >> 8u),
        (uint8_t)value,
        0x0D,
        0x0A,
    };

    memcpy(response->data, data, sizeof(data));
    response->length = sizeof(data);
}

void configuration_service_init(
    configuration_service_t *service,
    const runtime_config_t *defaults,
    uint32_t allowed_board_mask,
    const config_journal_storage_t *storage)
{
    if (service == NULL || !runtime_config_is_valid(defaults)) {
        return;
    }

    memset(service, 0, sizeof(*service));
    service->current = *defaults;
    service->allowed_board_mask = allowed_board_mask;
    config_journal_init(&service->journal, storage);
}

bool configuration_service_load(configuration_service_t *service)
{
    runtime_config_t loaded;

    if (service == NULL || !config_journal_load(&service->journal, &loaded) ||
        !board_is_allowed(service, loaded.board_id)) {
        return false;
    }

    service->current = loaded;
    service->loaded_from_flash = true;
    return true;
}

const runtime_config_t *configuration_service_current(
    const configuration_service_t *service)
{
    return service == NULL ? NULL : &service->current;
}

void configuration_service_handle(
    configuration_service_t *service,
    const uint8_t request[APP_PROTOCOL_REQUEST_SIZE],
    app_response_t *response)
{
    uint16_t value;
    bool valid;

    if (service == NULL || request == NULL || response == NULL) {
        return;
    }
    memset(response, 0, sizeof(*response));

    if (request[0] != CONFIG_PROTOCOL_HEADER) {
        return;
    }

    value = (uint16_t)(((uint16_t)request[3] << 8u) | request[4]);
    if (request[1] == APP_PROTOCOL_OPERATION_READ) {
        value = value_for_key(service, request[2], &valid);
        set_response(response,
            valid ? CONFIG_STATUS_OK : CONFIG_STATUS_INVALID_KEY,
            request[2], value);
        return;
    }

    if (request[1] != APP_PROTOCOL_OPERATION_WRITE) {
        set_response(response, CONFIG_STATUS_INVALID_VALUE, request[2], value);
        return;
    }

    switch (request[2]) {
        case CONFIG_KEY_BOARD_PROFILE:
            if (!board_is_allowed(service, (board_id_t)value)) {
                set_response(response, CONFIG_STATUS_INVALID_VALUE, request[2], value);
                return;
            }
            service->restart_required = service->current.board_id != (board_id_t)value;
            service->current.board_id = (board_id_t)value;
            break;
        case CONFIG_KEY_RESPONSE_DELAY_MS:
            if (value > RUNTIME_CONFIG_MAX_TIMING_MS) {
                set_response(response, CONFIG_STATUS_INVALID_VALUE, request[2], value);
                return;
            }
            service->current.response_delay_ms = value;
            break;
        case CONFIG_KEY_FRAME_TIMEOUT_MS:
            if (value == 0u || value > RUNTIME_CONFIG_MAX_TIMING_MS) {
                set_response(response, CONFIG_STATUS_INVALID_VALUE, request[2], value);
                return;
            }
            service->current.frame_timeout_ms = value;
            break;
        case CONFIG_KEY_SAVE:
            if (value != UINT16_C(0xA55A)) {
                set_response(response, CONFIG_STATUS_INVALID_VALUE, request[2], value);
                return;
            }
            if (!config_journal_save(&service->journal, &service->current)) {
                set_response(response, CONFIG_STATUS_STORAGE_ERROR, request[2], 0u);
                return;
            }
            value = 0u;
            break;
        default:
            set_response(response, CONFIG_STATUS_INVALID_KEY, request[2], value);
            return;
    }

    set_response(response, CONFIG_STATUS_OK, request[2], value);
}
