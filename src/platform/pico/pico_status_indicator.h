#ifndef PICO_STATUS_INDICATOR_H
#define PICO_STATUS_INDICATOR_H

#include "board_profile.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void pico_status_indicator_init(const board_profile_t *profile);
void pico_status_indicator_set(bool enabled);
void pico_status_indicator_toggle(void);
bool pico_status_indicator_is_available(void);

#ifdef __cplusplus
}
#endif

#endif

