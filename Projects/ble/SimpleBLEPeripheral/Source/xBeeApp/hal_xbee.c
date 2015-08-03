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

#if defined(HAL_XBEE_ZIGBEE)&& (HAL_XBEE_ZIGBEE == TRUE)
/***********************************************
**brief ≥ı ºªØIO
***********************************************/
void HalXbeeInit (void)
{
  //GPIO_XBEE_SLEEP_TURN_LOW();
  P0SEL &= 0x7F;
  GPIO_XBEE_SLEEP_DDR |= GPIO_XBEE_SLEEP_BV;
  GPIO_XBEE_SLEEP_TURN_LOW();
  
  GPIO_XBEE_RESET_TURN_HIGH();
  GPIO_XBEE_RTS_TURN_HIGH();
  P2SEL &= 0xFE;
  GPIO_XBEE_RESET_DDR |= GPIO_XBEE_RESET_BV;
  GPIO_XBEE_RTS_DDR |= GPIO_XBEE_RTS_BV;
  
  
}

uint8 HalGpioSet (uint8 pin, uint8 level)
{
  switch(pin)
  {
    case HAL_GPIO_XBEE_RESET:
      GPIO_XBEE_RESET_SBIT = level;
      break;
    case HAL_GPIO_XBEE_RTS:
      GPIO_XBEE_RTS_SBIT = level;
      break;
    default:
      break;
  }
  return 0;
}

uint8 HalGpioGet (uint8 pin)
{
  return 0;
}
#endif






















