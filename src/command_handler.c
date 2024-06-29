// command_handler.c

#include "command_handler.h"
#include "commands.h"
#include "tusb.h"
#include <stdlib.h>
#include <string.h>
#include "command_defs.h"
#include "htu21df.h"
#

const uint8_t RESP_ACK[] = {0xC3, 0x0D, 0x0A};
const uint8_t Write_ERROR_ACK[] = {0xC5, 0x0D, 0x0A};
uint8_t DA_RESP_TEMP[] = {0xFA, 0x02, 0x03, 0x00, 0x0D, 0x0A};
uint8_t DA_RESP_HUMIDITY[] =  {0xFA, 0x02, 0x03, 0x00, 0x0D, 0x0A};
uint8_t buf[2]=  {0xFA, 0x02};
float temp_value;
float humi_value;

void handle_command(const UHC_CMD *cmd) {

    response_t resp;

    resp.status = 0x00; // 默認狀態
    /**
     * @brief 判斷 head 的種類
     * 
     */
    switch (cmd->Head_type) {
        case CMD_HEAD_TYPE_FA:
            switch (cmd->UHC_Buffer[1]) { // 使用第三個字節來區分功能ID
                case FUNC_WRITE:                    
                    memcpy(resp.data, RESP_ACK, sizeof(RESP_ACK)); 
                    tud_cdc_write((const uint8_t*)&resp.data, 3u);
                    tud_cdc_write_flush();
                    break;
                case FUNC_READ:
                    // 處理功能1
                    break;
                // 添加更多功能處理
                default:
                    memcpy(resp.data, Write_ERROR_ACK, sizeof(Write_ERROR_ACK)); 
                    tud_cdc_write((const uint8_t*)&resp.data, 3u);
                    tud_cdc_write_flush();
                    break;
            }
            handle_type_fa(cmd, &resp);
            break;
        case CMD_HEAD_TYPE_DA:
            switch (cmd->UHC_Buffer[1]) { // 使用第三個字節來區分功能ID
                case FUNC_WRITE:
                    // 回傳 ACK
                    memcpy(resp.data, RESP_ACK, sizeof(RESP_ACK)); 
                    tud_cdc_write((const uint8_t*)&resp.data, 3u);
                    tud_cdc_write_flush();
                    break;
                case FUNC_READ:
                    switch (cmd->UHC_Buffer[3]) { // 使用第三個字節來區分功能ID
                        case DA_TEMP:   
                            if(true) {
                                
                                uint8_t temp_integer = (uint8_t)HTU21DF_readTemperature();
                                DA_RESP_TEMP[3] = temp_integer;
                                memcpy(resp.data, DA_RESP_TEMP, sizeof(DA_RESP_TEMP));
                                tud_cdc_write((const uint8_t*)&resp.data, sizeof(DA_RESP_TEMP));
                                tud_cdc_write_flush();  
                            }  

                            break;
                        case DA_HUMI:
                            if(true){

                                uint8_t temp_integer = HTU21DF_readHumidity();
                                DA_RESP_HUMIDITY[3] = temp_integer;
                                memcpy(resp.data, DA_RESP_HUMIDITY, sizeof(DA_RESP_HUMIDITY));
                                tud_cdc_write((const uint8_t*)&resp.data, sizeof(DA_RESP_HUMIDITY));
                                tud_cdc_write_flush();
                            }

                            break;
                        // 添加更多功能處理
                        default:
                            memcpy(resp.data, Write_ERROR_ACK, sizeof(Write_ERROR_ACK)); 
                            tud_cdc_write((const uint8_t*)&resp.data, 3u);
                            tud_cdc_write_flush();
                            break;
                    }
                    break;
                // 添加更多功能處理
                default:
                    memcpy(resp.data, Write_ERROR_ACK, sizeof(Write_ERROR_ACK)); 
                    tud_cdc_write((const uint8_t*)&resp.data, 3u);
                    tud_cdc_write_flush();
                    break;
            }

            handle_type_da(cmd, &resp);
            break;
        // 添加更多命令頭處理
        default:
            resp.status = 0xFF; // 未知命令頭
            break;
    }

}

void handle_temperature_request(response_t *resp) {
   
}
void handle_humidity_request(response_t *resp) {

}
void handle_type_fa(const UHC_CMD *cmd, response_t *resp) {

    switch (cmd->UHC_Buffer[1]) { // 使用第二個字節來區分設備ID
        case DEVICE_ID_1:
            // 根據功能ID處理
            switch (cmd->UHC_Buffer[2]) { // 使用第三個字節來區分功能ID
                case FA_INFO:
                    // 處理功能0
                    break;
                case FA_BRIGHTNESS:
                    // 處理功能1
                    break;
                // 添加更多功能處理
                default:
                    resp->status = 0xFF; // 未知功能
                    break;
            }
            break;
        case DEVICE_ID_2:
            // 處理設備2
            break;
        // 添加更多設備處理
        default:
            resp->status = 0xFF; // 未知設備
            break;
    }
}

void handle_type_da(const UHC_CMD *cmd, response_t *resp) {
    // 與handle_type_fa類似的處理
}

void response_usb_send(const void *data, size_t len) {
    const uint8_t *ptr = (const uint8_t *)data;
    uint32_t remaining = len;

    while (remaining > 0) {
        uint32_t sent = tud_cdc_write(ptr, remaining);
        ptr += sent;
        remaining -= sent;
        tud_task();
    }
    
    tud_cdc_write_flush();
}