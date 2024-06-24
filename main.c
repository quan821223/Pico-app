#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

#include "htu21df.h"
#include "hardware/i2c.h"

#include "ALL.h"
#include "tusb.h"
#include "bsp/board.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/unique_id.h"

#include "include/command_handler.h"
#include "include/commands.h"
#include "include/gpio.h"

#include "tud_cdc_descript.h"


//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+
/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum
{
    BLINK_NOT_MOUNTED = 250,
    BLINK_MOUNTED = 1000,
    BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void board_led_write(bool state);

void board_led_write(bool state)
{
    (void)state;

#ifdef LED_PIN
    gpio_put(LED_PIN, state ? LED_STATE_ON : (1 - LED_STATE_ON));
#endif
}

#define UART_ID uart0
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE
#define UART_TX_PIN 0
#define UART_RX_PIN 1
// 設置 Pin 26 和 Pin 27

#ifdef PICO_DEFAULT_LED_PIN
#define LED_PIN PICO_DEFAULT_LED_PIN
#endif

static int chars_rxed = 0;

uint8_t temp_len = 3;
uint8_t rx_buf_index = 0;
char rx_buf[64];
char return_buf[16];
char tud_cdc_rx_buffer[256];
bool return_request = false; // 存儲 tud_cdc_ReturnRequest() 的結果

void on_uart_rx()
{
    while (uart_is_readable(UART_ID))
    {
        uint8_t ch = uart_getc(UART_ID);
        if (uart_is_writable(UART_ID))
        {
            ch++;
            uart_putc(UART_ID, ch);
        }
        chars_rxed++;
    }
}

void init_uart(){
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);
    uart_set_fifo_enabled(UART_ID, false);
    uart_set_irq_enables(UART_ID, true, false);
    int __unused actual = uart_set_baudrate(UART_ID, BAUD_RATE);

    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);
}


int main()
{
    stdio_init_all();
    tusb_init();
    init_uart();
    init_gpio();
    HTU21DF_init();
    uint32_t last_led_toggle_time = 0;
    // LED init
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (!tud_mounted())
    {
        tud_task();
        sleep_ms(10);
    }
      
    while (1)
    {
        tud_task();
        tight_loop_contents(); // 簡單等待，讓中斷處理程序來處理按鈕事件

        if (tud_cdc_n_connected(2))
        {
            if (tud_cdc_available())
            {
                UHC_CMD cmd;
                uint32_t count = tud_cdc_read(tud_cdc_rx_buffer, sizeof(tud_cdc_rx_buffer));
                if (count > 0)
                {
                    cmd.UHC_Buffer =(uint8_t *)count;

                    if (cmd.UHC_Buffer != NULL) 
                    {
                        cmd.UHC_Buffer = tud_cdc_rx_buffer;  // 將 UHC_Buffer 設置為 tud_cdc_rx_buffer 的指標
                        memcpy(cmd.UHC_Buffer, tud_cdc_rx_buffer, count);  // 將接收到的數據複製到 UHC_Buffer 中
                        cmd.Buffer_Len = count;
                        cmd.Head_type = tud_cdc_rx_buffer[0];                        
                        handle_command(&cmd);
                        //free(cmd.UHC_Buffer); // 處理完畢後釋放內存
                    }

                }
                count = 0;
            }
        }

        led_blinking_task();
        // 如果需要返回訊息，則執行相應的操作
        if (return_request)
        {
            tud_cdc_read_flush();
            tud_cdc_turnoff();
            return_request = false; // 重置標誌為 false，以便下一次迭代
        }
        // 加入延遲，避免主程式死機
        sleep_ms(1);

        // 控制 LED 閃爍
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        if (current_time - last_led_toggle_time >= 500)
        {
            gpio_put(LED_PIN, !gpio_get(LED_PIN)); // 切換 LED 狀態
            last_led_toggle_time = current_time;
        }         
    }
}

//--------------------------------------------------------------------+
// Blinking Task
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
    const uint32_t interval_ms = 1000;
    static uint32_t start_ms = 0;

    static bool led_state = false;

    // Blink every interval ms
    if (board_millis() - start_ms < interval_ms)
        return; // not enough time
    start_ms += interval_ms;

    board_led_write(led_state);
    led_state = 1 - led_state; // toggle
}