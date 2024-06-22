/**
 * @file uart_bridge.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-09-18
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "include/uart_bridge.h"

/**
 * @brief
 *
 * @param uart
 */
static inline void UART_IsEnableACK(uart_inst_t *uart)
{
    // uart_write_blocking(uart, UART_Response, sizeof(UART_Response));
    if (uart_is_writable(uart))
        uart_write_blocking(uart, UART_Response, sizeof(UART_Response));
    // IsEnableACK = No;
}
