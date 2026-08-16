#include "runtime_config.h"

#include <stddef.h>

runtime_config_t runtime_config_defaults(board_id_t board_id)
{
    const runtime_config_t config = {
        .board_id = board_id,
        .response_delay_ms = RUNTIME_CONFIG_DEFAULT_RESPONSE_DELAY_MS,
        .frame_timeout_ms = RUNTIME_CONFIG_DEFAULT_FRAME_TIMEOUT_MS,
    };
    return config;
}

bool runtime_config_is_valid(const runtime_config_t *config)
{
    return config != NULL &&
        board_profile_get(config->board_id) != NULL &&
        config->response_delay_ms <= RUNTIME_CONFIG_MAX_TIMING_MS &&
        config->frame_timeout_ms > 0u &&
        config->frame_timeout_ms <= RUNTIME_CONFIG_MAX_TIMING_MS;
}
