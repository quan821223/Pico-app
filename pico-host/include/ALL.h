/**
 * @file ALL.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-09-16
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef ALL_H_
#define ALL_H_


#define MAX_RAND 0xFFFF
extern uint16_t last_rand;

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "uart_bridge.h"

// ----------------------------------------------------------------------------
// basic-specific operations and aliases
#define True 1u
#define False 0u
#define Highe 1u
#define Low 0u
#define Enable 1u
#define Disable 0u
#define On 1u
#define Off 0u

// FA 57 00 09 00/01
/**
 * @brief the flag is if the simulator is working.
 *
 */
bool FaultInjection;
bool IsWakeup;
uint8_t SDM_STATUS;
uint8_t SDM_Pattern;

int hundredth_min;
int hundredth_max;
int thousandth_min;
int thousandth_max;

int threshold_voltage;
int threshold_current;
int threshold_brightness;
int _seed;
int seed_voltage;
int seed_brightness;
int seed_current;

int min_n_brightness;
int max_n_brightness;
int min_s_brightness;
int max_s_brightness;

int min_s_voltage;
int max_s_voltage;
int min_n_voltage;
int max_n_voltage;

int min_s_current;
int max_s_current;
int min_n_current;
int max_n_current;

int Randnum_current;
int Randnum_Brightness;
int Randnum_voltage;
int Randnum_fingernum;
/**
 * @brief Receive data struct
 *
 */
typedef struct
{
    bool UART_fun_assert;
    bool UART_RX_flag;
    uint8_t UART_TXB_NUM;
    uint8_t UART_RxBuffer[256];
    uint8_t UART_ReceiveLength;

} UART_CMD_Type;
UART_CMD_Type UART_Sendor_CMD;

static void SDM_func_ACK();
static void SDM_func_SimulatorACK();
static void SDM_func_SDMSWVersion(uint8_t _type, uint8_t _TXBnum);
static void SDM_func_Diagnostic(uint8_t _type, uint8_t _TXBnum);
static void SDM_func_Brightness(uint8_t _TXBnum);
extern void SDM_func_Status(uint8_t _TXBnum);
extern void SDM_func_DisplayPattern(uint8_t _TXBnum);
static void SDM_func_Current(uint8_t _type, uint8_t _TXBnum);
extern void SDM_func_Touches(uint8_t _TXBnum);
extern int limited_rand( int min, int max);
extern int prob_based_rand(int current_value, int change_value, int probability) ;
extern void Execute_Simulator_CMD();



inline void UART_RX_INTB(void)
{

    while (uart_is_readable(UART_ID) && len_chars < sizeof(uart_buffer))
    {
        uart_buffer[len_chars++] = uart_getc(UART_ID);
    }

    if (len_chars > 0)
    {
        UART_Sendor_CMD.UART_ReceiveLength = len_chars;
        _id = 0;
        for (_id = 0; _id < len_chars; _id++)
        {
            UART_Sendor_CMD.UART_RxBuffer[_id] = *(uart_buffer + _id);
        }
        // uart_write_blocking(UART_ID, UART_Sendor_CMD.UART_RxBuffer, len_chars);
        UART_Sendor_CMD.UART_RX_flag = true;
        IsEnableACK = Yes;
    }
}

void Execute_Simulator_CMD()
{
    
    if (UART_Sendor_CMD.UART_RX_flag == true)
    {
        if (UART_Sendor_CMD.UART_RxBuffer[0] == 0xFA && UART_Sendor_CMD.UART_RxBuffer[1] == 0x52 && UART_Sendor_CMD.UART_ReceiveLength == 5)
        {

            // if(IsEnableACK)UART_IsEnableACK1(UART_ID);
            //  if(IsEnableACK)
            //  {
            //      SDM_SWver[1] = 0x44;
            //      uart_write_blocking(UART_ID, SDM_SWver, sizeof(SDM_SWver));
            //  }
            UART_Sendor_CMD.UART_TXB_NUM = UART_Sendor_CMD.UART_RxBuffer[2];
            switch (UART_Sendor_CMD.UART_RxBuffer[3])
            {
                case SDMVer:
                    SDM_func_SDMSWVersion(UART_Sendor_CMD.UART_RxBuffer[4], UART_Sendor_CMD.UART_TXB_NUM);
                    break;
                case Diagnostic:
                    SDM_func_Diagnostic(UART_Sendor_CMD.UART_RxBuffer[4], UART_Sendor_CMD.UART_TXB_NUM);
                    break;
                case Brightness:
                    SDM_func_Brightness(UART_Sendor_CMD.UART_TXB_NUM);
                    break;
                case Status:
                    SDM_func_Status(UART_Sendor_CMD.UART_TXB_NUM);
                    break;
                case DisplayPattern:
                    SDM_func_DisplayPattern(UART_Sendor_CMD.UART_TXB_NUM);
                    break;
                case Current:
                    SDM_func_Current(UART_Sendor_CMD.UART_RxBuffer[4], UART_Sendor_CMD.UART_TXB_NUM);
                    break;
                case Touches:
                    SDM_func_Touches(UART_Sendor_CMD.UART_TXB_NUM);
                    break;
                case Simulator:
                    SDM_func_SimulatorACK(UART_Sendor_CMD.UART_TXB_NUM);
                    break;
                default:
                    IsEnableACK = No;
                    len_chars = 0;
                    UART_Sendor_CMD.UART_RX_flag = No;
                    break;
            }
        }
        else if (UART_Sendor_CMD.UART_RxBuffer[0] == 0xFA && UART_Sendor_CMD.UART_RxBuffer[1] == 0x57 && UART_Sendor_CMD.UART_ReceiveLength == 5)
        {
            if (UART_Sendor_CMD.UART_RxBuffer[3] == 0x09 && UART_Sendor_CMD.UART_RxBuffer[4] == 0x01)
            {
                FaultInjection = 1;
            }
            else if (UART_Sendor_CMD.UART_RxBuffer[3] == 0x09 && UART_Sendor_CMD.UART_RxBuffer[4] == 0x00)
            {
                FaultInjection = 0;
            }
            else if (UART_Sendor_CMD.UART_RxBuffer[3] == 0x02  )
            {
                if (UART_Sendor_CMD.UART_RxBuffer[4] == 0x01 )
                {
                    IsWakeup = 1;
                }
                if (UART_Sendor_CMD.UART_RxBuffer[4] == 0x00 )
                {
                    IsWakeup = 0;
                }
            }          

            SDM_func_ACK();
        }
    }
    else
    {
        IsEnableACK = No;
        len_chars = 0;
        UART_Sendor_CMD.UART_RX_flag = No;
    }
}

static void SDM_func_ACK()
{
    uart_write_blocking(UART_ID, UART_Response1, sizeof(UART_Response1));
    UART_Sendor_CMD.UART_RX_flag = No;
}
static void SDM_func_SimulatorACK()
{
    if (FaultInjection)
        SDM_Simulator[4] = 0x01;
    else
        SDM_Simulator[4] = 0x00;
    uart_write_blocking(UART_ID, SDM_Simulator, sizeof(SDM_Simulator));

    UART_Sendor_CMD.UART_RX_flag = No;
}

static void SDM_func_SDMSWVersion(uint8_t _type, uint8_t _TXBnum)
{
// there are format 
// uint8_t SDM_SWver[7] = {0xFA, 0x01, 0x04, 0x01, 0x02, 0x0D, 0x0A};
// uint8_t SDM_HWver[7] = {0xFA, 0x01, 0x04, 0x03, 0x04, 0x0D, 0x0A};

    SDM_SWver[1] = _TXBnum;
    SDM_HWver[1] = _TXBnum;
    if (_type == SDMSWver)
        uart_write_blocking(UART_ID, SDM_SWver, sizeof(SDM_SWver));
    if (_type == SDMHWver)
        uart_write_blocking(UART_ID, SDM_HWver, sizeof(SDM_HWver));
    UART_Sendor_CMD.UART_RX_flag = No;
}

void splitIntIntoBytes(uint16_t value, uint8_t* highByte, uint8_t* lowByte) {
    *highByte = (value >> 8) & 0xFF;
    *lowByte = value & 0xFF;
}

void SDM_func_Diagnostic(uint8_t _type, uint8_t _TXBnum)
{
// uint8_t SDM_Diagnostic[14] = {0xFA, 0x01, 0x0A, 0x01,
                            //   0x01, 0x02, 0x03, 0x04, 
                            //   0x05, 0x06, 0x07, 0x08, 
                            //   0x0D, 0x0A}; 
   
    if (_type == GetDiagnostic)
    {
        SDM_Diagnostic[1] = _TXBnum;
        int new_value = prob_based_rand(1, 0, 1); // ???1%???æ©????å¾?0??¹è?????1
        SDM_Diagnostic[3] = new_value;
        uart_write_blocking(UART_ID, SDM_Diagnostic, sizeof(SDM_Diagnostic));
    }
    else if (_type == GetVoltageValue)
    {
// uint8_t SDM_Voltage[7]      = {0xFA, 0x01, 0x04, 0x00, 0x8A, 0x0D, 0x0A};  
        SDM_Voltage[1] = _TXBnum;
        uint16_t  newvoltage = limited_rand(133, 138); // ??????1348???1352ä¹?????????¨æ?????
        uint8_t high, low;

        splitIntIntoBytes(newvoltage, &high, &low);

        int new_value = prob_based_rand(0, 1, 1); // ???1%???æ©????å¾?0??¹è?????1
        SDM_Voltage[3] =  high;
        SDM_Voltage[4] =  low;
        uart_write_blocking(UART_ID, SDM_Voltage, sizeof(SDM_Voltage));
    }
    else if (_type == Gettemp)
    {
// uint8_t SDM_temp[6]     = {0xFA, 0x01, 0x05, 0x11, 0x11, 0x11, 0x0D, 0x0A};         
        SDM_temp[1] = _TXBnum;

        int LED1rangevalue = limited_rand(65, 70); // ??????65???70ä¹?????????¨æ?????
        int LED1new_value = prob_based_rand(0, 20, 1); // ???1%???æ©????å¾?value??¹è?????20
        SDM_temp[3] = (uint8_t) (LED1rangevalue+ LED1new_value);
        int LED2rangevalue = limited_rand(65, 70); // ??????65???70ä¹?????????¨æ?????
        int LED2new_value = prob_based_rand(0, 20, 1); // ???1%???æ©????å¾?value??¹è?????20
        SDM_temp[4] = (uint8_t) (LED2rangevalue+ LED2new_value);
        int PCBrangevalue = limited_rand(65, 70); // ??????65???70ä¹?????????¨æ?????
        int PCBnew_value = prob_based_rand(0, 20, 1); // ???2%???æ©????å¾?value??¹è?????20
        SDM_temp[5] = (uint8_t) (PCBrangevalue+ PCBnew_value);
        uart_write_blocking(UART_ID, SDM_temp, sizeof(SDM_temp));
    }
    else if (_type == GetAmbienttemp)
    {
        SDM_Ambienttemp[1] = _TXBnum;
        int rangevalue = limited_rand(75, 70); // ??????65???70ä¹?????????¨æ?????
        int new_value = prob_based_rand(0, 20, 1); // ???1%???æ©????å¾?value??¹è?????20
        SDM_Ambienttemp[3] = (uint8_t) (rangevalue+ new_value);
        uart_write_blocking(UART_ID, SDM_Ambienttemp, sizeof(SDM_Ambienttemp));

    }
    else if (_type == GetADC)
    {
//  uint8_t SDM_ADC[7]          = {0xFA, 0x01, 0x04, 0x11, 0x55, 0x0D, 0x0A};  
        SDM_ADC[1] = _TXBnum;
        int rangevalue1 = limited_rand(10, 80); // ??????65???70ä¹?????????¨æ?????
        int rangevalue2 = limited_rand(10, 80); // ??????65???70ä¹?????????¨æ?????
        SDM_ADC[3] = (uint8_t) rangevalue1;
        SDM_ADC[4] = (uint8_t) rangevalue2;
        uart_write_blocking(UART_ID, SDM_ADC, sizeof(SDM_ADC));
    }
    UART_Sendor_CMD.UART_RX_flag = No;
}

void SDM_func_Brightness(uint8_t _TXBnum)
{
//uint8_t SDM_BacklightBrightness[6] = {0xFA, 0x01, 0x03, 0x00, 0x0D, 0x0A};    
    SDM_BacklightBrightness[1] = _TXBnum;
    int value = 100;
    int new_value = prob_based_rand(value, 0, 2); // ???2%???æ©????å¾?value??¹è?????0
    SDM_BacklightBrightness[3] = new_value;
    uart_write_blocking(UART_ID, SDM_BacklightBrightness, sizeof(SDM_BacklightBrightness));
    UART_Sendor_CMD.UART_RX_flag = No;

}

void SDM_func_Status(uint8_t _TXBnum)
{
    uart_write_blocking(UART_ID, UART_Response1, sizeof(UART_Response1));
    UART_Sendor_CMD.UART_RX_flag = No;
}

void SDM_func_DisplayPattern(uint8_t _TXBnum)
{
    uart_write_blocking(UART_ID, UART_Response1, sizeof(UART_Response1));
    UART_Sendor_CMD.UART_RX_flag = No;
}

void SDM_func_Current(uint8_t _type, uint8_t _TXBnum)
{
// uint8_t SDM_NormCurrent[6] = {0xFA, 0x01, 0x04, 0x12, 0x34,0x0D, 0x0A}; 
// uint8_t SDM_SleepCurrent[6] = {0xFA, 0x01, 0x04, 0x00, 0x34,0x0D, 0x0A}; 
    SDM_NormCurrent[1] = _TXBnum;
    SDM_SleepCurrent[1] = _TXBnum;
    if (IsWakeup == 1)
    {
        int value = 1;
        int new_value = prob_based_rand(value, 5, 1); // ???1%???æ©????å¾?value??¹è?????5
        SDM_NormCurrent[3] = new_value;
        SDM_NormCurrent[4] = limited_rand(0, 99); // ??????10???50ä¹?????????¨æ?????
       
        uart_write_blocking(UART_ID, SDM_NormCurrent, sizeof(SDM_NormCurrent));
        UART_Sendor_CMD.UART_RX_flag = No;
    }
    else if (IsWakeup == 0)
    {
           int value = 1;
        int new_value = prob_based_rand(value, 30, 1); // ???1%???æ©????å¾?value??¹è?????30
        SDM_SleepCurrent[4] = limited_rand(0, 10) + new_value; // ??????0???99ä¹?????????¨æ?????
        uart_write_blocking(UART_ID, SDM_SleepCurrent, sizeof(SDM_SleepCurrent));
        UART_Sendor_CMD.UART_RX_flag = No;
        
    }       
    
}

void SDM_func_Touches(uint8_t _TXBnum)
{
//    uint8_t SDM_Touch[10] = {0xFA, 0x01, 0x07, 0x01, 0x01, 
//                             0x00, 0x32, 0x32, 0x0D, 0x0A}; // 3 
    SDM_Touch[1] = _TXBnum;
    SDM_Touch[6] = prob_based_rand(32, 10, 1);
    uart_write_blocking(UART_ID, SDM_Touch, sizeof(SDM_Touch));
    UART_Sendor_CMD.UART_RX_flag = No;
}



int limited_rand( int min, int max) {
    last_rand = (18000U * (last_rand & 0xFFFFU)) + (last_rand >> 16);
    return (last_rand % (max - min + 1)) + min;
}

int prob_based_rand(int current_value, int change_value, int probability) {
    int rand_val = limited_rand(1, 100); // 1???100?????¨æ?????
    if(rand_val <= probability) {
        return change_value;
    } else {
        return current_value;
    }
}



#endif