#include "board_profile.h"

#include <stddef.h>

#ifndef APP_ACTIVE_BOARD_ID
#define APP_ACTIVE_BOARD_ID BOARD_ID_PICO
#endif

static const board_profile_t PROFILES[BOARD_ID_COUNT] = {
    [BOARD_ID_PICO] = {
        .id = BOARD_ID_PICO,
        .name = "Raspberry Pi Pico",
        .pico_sdk_board = "pico",
        .mcu_family = BOARD_MCU_RP2040,
        .flash_size_bytes = 2u * 1024u * 1024u,
        .debug_uart_index = 0u,
        .debug_uart_tx_pin = 16u,
        .debug_uart_rx_pin = 17u,
        .chamber_output_0_pin = 2u,
        .chamber_output_1_pin = 3u,
        .chamber_address_0_pin = 18u,
        .chamber_address_1_pin = 19u,
        .touch_output_0_pin = 20u,
        .touch_output_1_pin = 21u,
        .indicator_kind = BOARD_INDICATOR_GPIO,
        .indicator_pin = 25u,
    },
    [BOARD_ID_PICO_W] = {
        .id = BOARD_ID_PICO_W,
        .name = "Raspberry Pi Pico W",
        .pico_sdk_board = "pico_w",
        .mcu_family = BOARD_MCU_RP2040,
        .flash_size_bytes = 2u * 1024u * 1024u,
        .debug_uart_index = 0u,
        .debug_uart_tx_pin = 16u,
        .debug_uart_rx_pin = 17u,
        .chamber_output_0_pin = 2u,
        .chamber_output_1_pin = 3u,
        .chamber_address_0_pin = 18u,
        .chamber_address_1_pin = 19u,
        .touch_output_0_pin = 20u,
        .touch_output_1_pin = 21u,
        .indicator_kind = BOARD_INDICATOR_CYW43,
        .indicator_pin = BOARD_PIN_UNUSED,
    },
    [BOARD_ID_WAVESHARE_RP2040_ZERO] = {
        .id = BOARD_ID_WAVESHARE_RP2040_ZERO,
        .name = "Waveshare RP2040-Zero",
        .pico_sdk_board = "waveshare_rp2040_zero",
        .mcu_family = BOARD_MCU_RP2040,
        .flash_size_bytes = 2u * 1024u * 1024u,
        .debug_uart_index = 0u,
        .debug_uart_tx_pin = 0u,
        .debug_uart_rx_pin = 1u,
        .chamber_output_0_pin = 2u,
        .chamber_output_1_pin = 3u,
        .chamber_address_0_pin = 18u,
        .chamber_address_1_pin = 19u,
        .touch_output_0_pin = 20u,
        .touch_output_1_pin = 21u,
        .indicator_kind = BOARD_INDICATOR_WS2812,
        .indicator_pin = 16u,
    },
    [BOARD_ID_PICO2] = {
        .id = BOARD_ID_PICO2,
        .name = "Raspberry Pi Pico 2",
        .pico_sdk_board = "pico2",
        .mcu_family = BOARD_MCU_RP2350,
        .flash_size_bytes = 4u * 1024u * 1024u,
        .debug_uart_index = 0u,
        .debug_uart_tx_pin = 16u,
        .debug_uart_rx_pin = 17u,
        .chamber_output_0_pin = 2u,
        .chamber_output_1_pin = 3u,
        .chamber_address_0_pin = 18u,
        .chamber_address_1_pin = 19u,
        .touch_output_0_pin = 20u,
        .touch_output_1_pin = 21u,
        .indicator_kind = BOARD_INDICATOR_GPIO,
        .indicator_pin = 25u,
    },
};

const board_profile_t *board_profile_get(board_id_t id)
{
    return (unsigned int)id < (unsigned int)BOARD_ID_COUNT
        ? &PROFILES[id]
        : NULL;
}

const board_profile_t *board_profile_active(void)
{
    return board_profile_get((board_id_t)APP_ACTIVE_BOARD_ID);
}

bool board_profile_is_valid(const board_profile_t *profile)
{
    if (profile == NULL || profile->name == NULL ||
        profile->pico_sdk_board == NULL) {
        return false;
    }

    if (profile->debug_uart_index > 1u ||
        profile->debug_uart_tx_pin == profile->debug_uart_rx_pin ||
        profile->flash_size_bytes == 0u) {
        return false;
    }

    if (profile->indicator_kind == BOARD_INDICATOR_GPIO &&
        profile->indicator_pin == BOARD_PIN_UNUSED) {
        return false;
    }

    return true;
}
