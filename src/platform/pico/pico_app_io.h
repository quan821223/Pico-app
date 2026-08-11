#ifndef PICO_APP_IO_H
#define PICO_APP_IO_H

#include "app_protocol.h"
#include "board_profile.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

bool pico_app_io_init(const board_profile_t *profile);
uint8_t pico_app_io_read_chamber_status(void);
void pico_app_io_apply_effect(const app_effect_t *effect);

void pico_app_io_set_chamber_backdoor_enabled(bool enabled);
bool pico_app_io_set_chamber_backdoor_value(uint8_t value);

#ifdef __cplusplus
}
#endif

#endif
