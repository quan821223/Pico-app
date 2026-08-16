#ifndef RUNTIME_CONFIG_H
#define RUNTIME_CONFIG_H

#include "board_profile.h"

#include <stdbool.h>
#include <stdint.h>

#define RUNTIME_CONFIG_DEFAULT_RESPONSE_DELAY_MS 50u
#define RUNTIME_CONFIG_DEFAULT_FRAME_TIMEOUT_MS 100u
#define RUNTIME_CONFIG_MAX_TIMING_MS 5000u

typedef struct {
    board_id_t board_id;
    uint16_t response_delay_ms;
    uint16_t frame_timeout_ms;
} runtime_config_t;

runtime_config_t runtime_config_defaults(board_id_t board_id);
bool runtime_config_is_valid(const runtime_config_t *config);

#endif
