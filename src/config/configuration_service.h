#ifndef CONFIGURATION_SERVICE_H
#define CONFIGURATION_SERVICE_H

#include "app_protocol.h"
#include "config_journal.h"
#include "runtime_config.h"

#include <stdbool.h>
#include <stdint.h>

#define CONFIG_PROTOCOL_HEADER 0xCFu
#define CONFIG_KEY_BOARD_PROFILE 0x01u
#define CONFIG_KEY_RESPONSE_DELAY_MS 0x02u
#define CONFIG_KEY_FRAME_TIMEOUT_MS 0x03u
#define CONFIG_KEY_SAVE 0x7Eu

typedef struct {
    runtime_config_t current;
    config_journal_t journal;
    uint32_t allowed_board_mask;
    bool loaded_from_flash;
    bool restart_required;
} configuration_service_t;

void configuration_service_init(
    configuration_service_t *service,
    const runtime_config_t *defaults,
    uint32_t allowed_board_mask,
    const config_journal_storage_t *storage);

bool configuration_service_load(configuration_service_t *service);

const runtime_config_t *configuration_service_current(
    const configuration_service_t *service);

void configuration_service_handle(
    configuration_service_t *service,
    const uint8_t request[APP_PROTOCOL_REQUEST_SIZE],
    app_response_t *response);

#endif
