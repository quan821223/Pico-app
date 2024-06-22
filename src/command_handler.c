// command_handler.c
#include "command_handler.h"
#include "tusb.h"

void handle_command(const command_t *cmd) {
    response_t resp;
    resp.status = 0x00; // 默認狀態

    switch (cmd->type) {
        case CMD_TYPE_READ:
            // 根據指令參數處理讀取邏輯
            resp.data[0] = 0x01; // 模擬讀取數據
            break;
        case CMD_TYPE_WRITE:
            // 根據指令參數處理寫入邏輯
            resp.status = 0x01; // 模擬成功狀態
            break;
        default:
            resp.status = 0xFF; // 未知指令
            break;
    }

    tud_cdc_write((const uint8_t*)&resp, sizeof(resp));
    tud_cdc_write_flush();
}