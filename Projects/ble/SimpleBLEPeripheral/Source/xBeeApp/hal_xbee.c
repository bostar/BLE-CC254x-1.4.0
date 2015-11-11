/**********************************************************
**brief 
**
**
**
**
**********************************************************/

#include "hal_mcu.h"
#include "hal_defs.h"
#include "hal_types.h"
#include "hal_drivers.h"
#include "hal_zlg.h"
#include "osal.h"
#include "hal_board.h"
#include "XBeeAtCmd.h"
#include "hal_xbee.h"
#include "hal_board_cfg.h"

#if defined(HAL_XBEE_ZIGBEE)&& (HAL_XBEE_ZIGBEE == TRUE)
/***********************************************
**brief ≥ı ºªØIO
***********************************************/
void HalXbeeInit (void)
{          
    GPIO_XBEE_RTS_DDR           |=  GPIO_XBEE_RTS_BV;           
    GPIO_XBEE_RESET_DDR         |=  GPIO_XBEE_RESET_BV;  
	GPIO_XBEE_SLEEP_DDR         |=  GPIO_XBEE_SLEEP_BV;  
	GPIO_XBEE_MOTOR1_DDR        |=  GPIO_XBEE_MOTOR1_BV;
	GPIO_XBEE_MOTOR2_DDR        |=  GPIO_XBEE_MOTOR2_BV;
    GPIO_XBEE_SLEEP_INDER_DDR   &=  ~GPIO_XBEE_SLEEP_INDER_BV;
	GPIO_XBEE_CTS_DDR           &=  ~GPIO_XBEE_CTS_BV;  
	GPIO_XBEE_KEY1_DDR          &=  ~GPIO_XBEE_KEY1_BV;  
	GPIO_XBEE_KEY2_DDR          &=  ~GPIO_XBEE_KEY2_BV;  
    GPIO_XBEE_KEY3_DDR          &=  ~GPIO_XBEE_KEY3_BV;  
    GPIO_XBEE_MOTOR_SENSER_DDR  &=  ~GPIO_XBEE_MOTOR_SENSER_BV;
    GPIO_XBEE_VBT_SENSER_SBIT   &=  ~GPIO_XBEE_VBT_SENSER_BV;
    GPIO_XBEE_DIR_DDR |= GPIO_XBEE_DIR_BV;
    
    GPIO_XBEE_RESET_TURN_LOW();
    HAL_GPIO_CHANGE_DELAY();
    GPIO_XBEE_RESET_TURN_HIGH();   
    GPIO_XBEE_RTS_TURN_LOW();
    GPIO_XBEE_SLEEP_TURN_HIGH();
    //GPIO_XBEE_SLEEP_TURN_LOW();
    GPIO_XBEE_DIR_TURN_LOW();
    GPIO_XBEE_MOTOR1_TURN_HIGH();
    GPIO_XBEE_MOTOR2_TURN_HIGH();
}

uint8 HalGpioGet(GPIOType pin)
{
    uint8 reval;
    switch(pin)
    {
        case GPIO_XBEE_KEY1:
            reval = GPIO_XBEE_KEY1_SBIT;
            break;
        case GPIO_XBEE_KEY2:
            reval = GPIO_XBEE_KEY2_SBIT;
            break;
        case GPIO_XBEE_KEY3:
            reval = GPIO_XBEE_KEY3_SBIT;
            break;
        case GPIO_XBEE_SLEEP_INDER:
            reval = GPIO_XBEE_SLEEP_INDER_SBIT;
            break;
        case GPIO_XBEE_MOTOR1:
            reval = GPIO_XBEE_MOTOR1_SBIT;
            break;
        case GPIO_XBEE_MOTOR2:
            reval = GPIO_XBEE_MOTOR2_SBIT;
            break;
        case GPIO_XBEE_CTS:
            reval = GPIO_XBEE_CTS_SBIT;
            break;
        default:
            break;
    }
    return reval;
}
#endif






















