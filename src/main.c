#include "app_protocol.h"
#include "board_profile.h"
#include "configuration_service.h"
#include "pico_app_io.h"
#include "pico_config_storage.h"
#include "pico_status_indicator.h"
#include "protocol_service.h"

#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"
#include "tusb.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

enum {
    LED_FLASH_INTERVAL_MS = 500u,
    DEBUG_UART_BAUD_RATE = 115200u,
    BACKDOOR_PACKET_SIZE = 5u,
};

typedef struct {
    absolute_time_t trigger_time;
    uint8_t data[APP_PROTOCOL_MAX_RESPONSE_SIZE];
    size_t length;
    bool pending;
} delayed_response_t;

static const uint8_t UART_ACK[] = {0xC3, 0x0D, 0x0A};
static const uint8_t INVALID_FRAME_RESPONSE[] = {
    0xEC, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x0A,
};
static const board_profile_t *active_board;
static uart_inst_t *debug_uart;
static protocol_service_t protocol_service;
static configuration_service_t configuration_service;
static delayed_response_t delayed_response;
static uint8_t uart_backdoor_buffer[BACKDOOR_PACKET_SIZE];
static size_t uart_backdoor_length;

static void debug_uart_write(const uint8_t *data, size_t length)
{
    for (size_t index = 0; index < length; ++index) {
        uart_putc(debug_uart, data[index]);
    }
}
static void usb_send(const uint8_t *data, size_t length)
{
    while (length > 0u) {
        const uint32_t available = tud_cdc_write_available();

        if (available > 0u) {
            const size_t chunk = length < available ? length : available;
            const size_t written = tud_cdc_write(data, chunk);

            debug_uart_write(data, written);
            data += written;
            length -= written;
        }
        tud_task();
    }
    tud_cdc_write_flush();
}

static uint8_t read_chamber_status(void *context)
{
    (void)context;
    return pico_app_io_read_chamber_status();
}

static void apply_effect(const app_effect_t *effect, void *context)
{
    (void)context;
    pico_app_io_apply_effect(effect);
}

static uint16_t read_response_delay_ms(void *context)
{
    const runtime_config_t *config;
    (void)context;

    config = configuration_service_current(&configuration_service);
    return config == NULL
        ? RUNTIME_CONFIG_DEFAULT_RESPONSE_DELAY_MS
        : config->response_delay_ms;
}

static void handle_control(
    const uint8_t request[APP_PROTOCOL_REQUEST_SIZE],
    app_response_t *response,
    void *context)
{
    (void)context;
    configuration_service_handle(&configuration_service, request, response);
}

static void schedule_response(
    const uint8_t *data,
    size_t length,
    uint32_t delay_ms,
    void *context)
{
    (void)context;

    if (data == NULL || length > sizeof(delayed_response.data)) {
        return;
    }

    memcpy(delayed_response.data, data, length);
    delayed_response.length = length;
    delayed_response.trigger_time = make_timeout_time_ms(delay_ms);
    delayed_response.pending = true;
}

static bool process_uart_backdoor_packet(const uint8_t *packet)
{
    if (packet[0] != APP_PROTOCOL_HEADER_DA ||
        packet[1] != APP_PROTOCOL_OPERATION_WRITE ||
        packet[2] != 0x03u) {
        return false;
    }

    if (packet[3] == 0xFFu) {
        pico_app_io_set_chamber_backdoor_enabled(packet[4] == 0x01u);
        return true;
    }

    if (packet[3] == 0x0Au) {
        return pico_app_io_set_chamber_backdoor_value(packet[4]);
    }

    return false;
}

static void poll_uart_backdoor(void)
{
    while (uart_is_readable(debug_uart)) {
        const uint8_t byte = uart_getc(debug_uart);

        if (byte == APP_PROTOCOL_HEADER_DA) {
            uart_backdoor_length = 0u;
        } else if (uart_backdoor_length == 0u) {
            continue;
        }

        uart_backdoor_buffer[uart_backdoor_length++] = byte;
        if (uart_backdoor_length == sizeof(uart_backdoor_buffer)) {
            if (process_uart_backdoor_packet(uart_backdoor_buffer)) {
                debug_uart_write(UART_ACK, sizeof(UART_ACK));
            }
            uart_backdoor_length = 0u;
        }
    }
}

static bool initialize_debug_uart(const board_profile_t *profile)
{
    if (!board_profile_is_valid(profile)) {
        return false;
    }

    debug_uart = profile->debug_uart_index == 0u ? uart0 : uart1;
    uart_init(debug_uart, DEBUG_UART_BAUD_RATE);
    gpio_set_function(profile->debug_uart_tx_pin, GPIO_FUNC_UART);
    gpio_set_function(profile->debug_uart_rx_pin, GPIO_FUNC_UART);
    return true;
}

static void initialize_protocol(void)
{
    const protocol_service_ports_t ports = {
        .read_chamber_status = read_chamber_status,
        .apply_effect = apply_effect,
        .response_ready = schedule_response,
        .read_response_delay_ms = read_response_delay_ms,
        .handle_control = handle_control,
        .context = NULL,
    };

    protocol_service_init(&protocol_service, &ports);
}

void tud_cdc_rx_cb(uint8_t interface_number)
{
    uint8_t buffer[64];
    const uint32_t count = tud_cdc_n_read(
        interface_number, buffer, sizeof(buffer));

    if (count == 0u) {
        return;
    }

    pico_status_indicator_set(true);
    debug_uart_write(buffer, count);
    protocol_service_feed(
        &protocol_service,
        buffer,
        count,
        to_ms_since_boot(get_absolute_time()));
    pico_status_indicator_set(false);
}

int main(void)
{
    uint32_t last_led_toggle = 0u;
    runtime_config_t defaults;
    config_journal_storage_t config_storage;
    uint32_t allowed_board_mask;
    const runtime_config_t *config;

    stdio_init_all();
    active_board = board_profile_active();
    if (!board_profile_is_valid(active_board)) {
        return 1;
    }

    defaults = runtime_config_defaults(active_board->id);
    allowed_board_mask = active_board->mcu_family == BOARD_MCU_RP2040
        ? (UINT32_C(1) << BOARD_ID_PICO) |
            (UINT32_C(1) << BOARD_ID_PICO_W) |
            (UINT32_C(1) << BOARD_ID_WAVESHARE_RP2040_ZERO)
        : (UINT32_C(1) << BOARD_ID_PICO2);
    config_storage = pico_config_storage();
    configuration_service_init(
        &configuration_service,
        &defaults,
        allowed_board_mask,
        &config_storage);
    configuration_service_load(&configuration_service);
    config = configuration_service_current(&configuration_service);
    active_board = config == NULL
        ? board_profile_active()
        : board_profile_get(config->board_id);

    if (!board_profile_is_valid(active_board) ||
        !initialize_debug_uart(active_board) ||
        !pico_app_io_init(active_board)) {
        return 1;
    }
    pico_status_indicator_init(active_board);
    initialize_protocol();
    tusb_init();

    while (true) {
        const uint32_t now_ms = to_ms_since_boot(get_absolute_time());

        tud_task();
        poll_uart_backdoor();

        config = configuration_service_current(&configuration_service);
        if (config != NULL && protocol_service_expire(
                &protocol_service, now_ms, config->frame_timeout_ms)) {
            schedule_response(
                INVALID_FRAME_RESPONSE,
                sizeof(INVALID_FRAME_RESPONSE),
                0u,
                NULL);
        }

        if ((uint32_t)(now_ms - last_led_toggle) >= LED_FLASH_INTERVAL_MS) {
            pico_status_indicator_toggle();
            last_led_toggle = now_ms;
        }

        if (delayed_response.pending &&
            absolute_time_diff_us(
                get_absolute_time(), delayed_response.trigger_time) <= 0) {
            usb_send(delayed_response.data, delayed_response.length);
            delayed_response.pending = false;
        }
    }
}
