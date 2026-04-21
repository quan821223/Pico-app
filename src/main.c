#include "pico/stdlib.h"
#include "tusb.h"
#include "tud_cdc_descript.h"
#include "ALL.h"
DelayedResponse DelayResponse = {0};
#include "hardware/uart.h"
#include "hardware/gpio.h"
#define LED_PIN 25
#define LED_FLASH_INTERVAL_MS 500
#define UART_ID uart0
#define UART_TX_PIN 16
#define UART_RX_PIN 17  // 若只做輸出可省略
#define BAUD_RATE 115200
#define BACKDOOR_PACKET_LEN 5



// 固定回應
const uint8_t RESPONSE[] = {0xC3, 0x0D, 0x0A};
static const uint8_t UART_ACK[] = {0xC3, 0x0D, 0x0A};
static uint8_t uart_backdoor_buffer[BACKDOOR_PACKET_LEN];
static uint8_t uart_backdoor_index = 0;

static void uart_send_ack(void) {
    for (size_t i = 0; i < sizeof(UART_ACK); ++i) {
        uart_putc(UART_ID, UART_ACK[i]);
    }
}

static bool process_uart_backdoor_packet(const uint8_t *packet) {
    if (packet[0] != Header_TYPE_DA || packet[1] != MSG_TYPE_WRITE || packet[2] != DEVICE_TYPE_3) {
        return false;
    }

    if (packet[3] == 0xFF) {
        chamber_status_backdoor_set_enabled(packet[4] == 0x01);
        return true;
    } else if (packet[3] == 0x0A) {
        chamber_status_backdoor_set_value(packet[4]);
        return packet[4] <= 0x03;
    }

    return false;
}

static void poll_uart_backdoor(void) {
    while (uart_is_readable(UART_ID)) {
        uint8_t byte = uart_getc(UART_ID);

        if (uart_backdoor_index == 0 && byte != Header_TYPE_DA) {
            continue;
        }

        if (byte == Header_TYPE_DA) {
            uart_backdoor_index = 0;
        }

        uart_backdoor_buffer[uart_backdoor_index++] = byte;

        if (uart_backdoor_index == BACKDOOR_PACKET_LEN) {
            if (process_uart_backdoor_packet(uart_backdoor_buffer)) {
                uart_send_ack();
            }
            uart_backdoor_index = 0;
        }
    }
}

// USB 發送函數
void usb_send(const uint8_t* data, size_t len) {
    while (len > 0) {
        uint32_t available = tud_cdc_write_available();
        if (available) {
            size_t to_write = (len < available) ? len : available;
            size_t written = tud_cdc_write(data, to_write);

            // mirror to UART1
            for (size_t i = 0; i < written; ++i) {
                uart_putc(UART_ID, data[i]);
            }

            data += written;
            len -= written;
        }
        tud_task();
    }
    tud_cdc_write_flush();
}

// USB CDC 接收回調
void tud_cdc_rx_cb(uint8_t itf) {
    uint8_t buf[64];
    uint32_t count = tud_cdc_read(buf, sizeof(buf));
    
    if (count > 0) {
        // 收到數據時 LED 亮起
        gpio_put(LED_PIN, 1);
        
        // UART mirror output
        for (uint32_t i = 0; i < count; ++i) {
            uart_putc(UART_ID, buf[i]);
        }


        receive_data(buf, count);
        // LED 熄滅
        gpio_put(LED_PIN, 0);
    }
}
void chamber_init(void) {
    gpio_init(GPIO_PIN2);
    gpio_set_dir(GPIO_PIN2, GPIO_OUT);
    gpio_put(GPIO_PIN2, 1);

    gpio_init(GPIO_PIN3);
    gpio_set_dir(GPIO_PIN3, GPIO_OUT);
    gpio_put(GPIO_PIN3, 1);

    // Two-bit hardware address input (default high, short to GND => low)
    gpio_init(CHAMBER_ADDR_PIN0);
    gpio_set_dir(CHAMBER_ADDR_PIN0, GPIO_IN);
    gpio_pull_up(CHAMBER_ADDR_PIN0);

    gpio_init(CHAMBER_ADDR_PIN1);
    gpio_set_dir(CHAMBER_ADDR_PIN1, GPIO_IN);
    gpio_pull_up(CHAMBER_ADDR_PIN1);
}

int main() {
    // 初始化系統
    stdio_init_all();
    
    // 初始化 LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    // 初始化 UART1
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART); // 可省略若只 TX
    
    chamber_init();

    // 初始化 USB
    tusb_init();
    
    // 上次 LED 翻轉的時間
    uint32_t last_led_toggle = 0;
    
    while (1) {
        tud_task();  // USB 任務處理
        poll_uart_backdoor();
        
        // LED 閃爍
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_led_toggle >= LED_FLASH_INTERVAL_MS) {
            gpio_put(LED_PIN, !gpio_get(LED_PIN));
            last_led_toggle = now;
        }
        if (DelayResponse.valid && absolute_time_diff_us(get_absolute_time(), DelayResponse.trigger_time) <= 0) {
                usb_send(DelayResponse.buffer, DelayResponse.length);
                DelayResponse.valid = false;
            }
    }
}
