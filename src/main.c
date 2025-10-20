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



// 固定回應
const uint8_t RESPONSE[] = {0xC3, 0x0D, 0x0A};

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
    // 這些程式碼從 main.c 搬過來
    gpio_init(GPIO_PIN2);
    gpio_set_dir(GPIO_PIN2, GPIO_OUT);
    gpio_put(GPIO_PIN2, 1); // 假設預設狀態是 ON

    gpio_init(GPIO_PIN3);
    gpio_set_dir(GPIO_PIN3, GPIO_OUT);
    gpio_put(GPIO_PIN3, 1); // 假設預設狀態是 ON
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

    gpio_init(GPIO_PIN2);
    gpio_set_dir(GPIO_PIN2, GPIO_OUT);
    gpio_put(GPIO_PIN2, 1);
    gpio_init(GPIO_PIN3);
    gpio_set_dir(GPIO_PIN3, GPIO_OUT);
    gpio_put(GPIO_PIN3, 1);
    // 初始化 USB
    tusb_init();
    
    // 上次 LED 翻轉的時間
    uint32_t last_led_toggle = 0;
    
    while (1) {
        tud_task();  // USB 任務處理
        
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