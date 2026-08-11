#ifndef BOARD_PROFILE_H
#define BOARD_PROFILE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BOARD_PIN_UNUSED 0xFFu

typedef enum {
    BOARD_ID_PICO = 0,
    BOARD_ID_PICO_W = 1,
    BOARD_ID_WAVESHARE_RP2040_ZERO = 2,
    BOARD_ID_PICO2 = 3,
    BOARD_ID_COUNT,
} board_id_t;

typedef enum {
    BOARD_MCU_RP2040 = 0,
    BOARD_MCU_RP2350,
} board_mcu_family_t;

typedef enum {
    BOARD_INDICATOR_NONE = 0,
    BOARD_INDICATOR_GPIO,
    BOARD_INDICATOR_WS2812,
    BOARD_INDICATOR_CYW43,
} board_indicator_kind_t;

typedef struct {
    board_id_t id;
    const char *name;
    const char *pico_sdk_board;
    board_mcu_family_t mcu_family;
    uint32_t flash_size_bytes;
    uint8_t debug_uart_index;
    uint8_t debug_uart_tx_pin;
    uint8_t debug_uart_rx_pin;
    uint8_t chamber_output_0_pin;
    uint8_t chamber_output_1_pin;
    uint8_t chamber_address_0_pin;
    uint8_t chamber_address_1_pin;
    uint8_t touch_output_0_pin;
    uint8_t touch_output_1_pin;
    board_indicator_kind_t indicator_kind;
    uint8_t indicator_pin;
} board_profile_t;

const board_profile_t *board_profile_get(board_id_t id);
const board_profile_t *board_profile_active(void);
bool board_profile_is_valid(const board_profile_t *profile);

#ifdef __cplusplus
}
#endif

#endif

