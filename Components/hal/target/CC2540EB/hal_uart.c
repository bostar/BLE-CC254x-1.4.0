/**************************************************************************************************
  Filename:     hal_uart.c
  Revised:      $Date: 2013-02-06 09:21:21 -0800 (Wed, 06 Feb 2013) $
  Revision:     $Revision: 33001 $

  Description:  This file contains the interface to the H/W UART driver.


  Copyright 2006-2012 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */

#include "hal_board_cfg.h"
#include "hal_defs.h"
#include "hal_drivers.h"
#include "hal_types.h"
#include "hal_uart.h"
#if defined POWER_SAVING
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#endif

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

void HalUARTIsrDMA(void);

/*********************************************************************
 * LOCAL FUNCTIONS
 */

#if HAL_UART_DMA
#include "_hal_uart_dma.c"
#endif
#if HAL_UART_ISR
#include "_hal_uart_isr.c"
#endif
#if HAL_UART_SPI
#include "_hal_uart_spi.c"
#endif
#if HAL_UART_USB
#include "_hal_uart_usb.c"
#endif

#include "XBeeApp.h"

/******************************************************************************
 * @fn      HalUARTInit
 *
 * @brief   Initialize the UART
 *
 * @param   none
 *
 * @return  none
 *****************************************************************************/
void HalUARTInit(void)
{
#if (HAL_UART_DMA && HAL_UART_SPI)  // When both are defined, port is run-time choice.
  if (HAL_UART_PORT)
  {
    HalUARTInitSPI();
  }
  else
  {
    HalUARTInitDMA();
  }
#else
#if HAL_UART_DMA
  HalUARTInitDMA();
#endif
#if HAL_UART_ISR
  HalUARTInitISR();
#endif
#if HAL_UART_SPI
  HalUARTInitSPI();
#endif
#if HAL_UART_USB
  HalUARTInitUSB();
#endif
#endif
}

/******************************************************************************
 * @fn      HalUARTOpen
 *
 * @brief   Open a port according tp the configuration specified by parameter.
 *
 * @param   port   - UART port
 *          config - contains configuration information
 *
 * @return  Status of the function call
 *****************************************************************************/
uint8 HalUARTOpen(uint8 port, halUARTCfg_t *config)
{
#if (HAL_UART_DMA == 1)
  if (port == HAL_UART_PORT_0)  HalUARTOpenDMA(config);
#endif
#if (HAL_UART_DMA == 2)
  if (port == HAL_UART_PORT_1)  HalUARTOpenDMA(config);
#endif
#if (HAL_UART_ISR == 1)
  if (port == HAL_UART_PORT_0)  HalUARTOpenISR(config);
#endif
#if (HAL_UART_ISR == 2)
  if (port == HAL_UART_PORT_1)  HalUARTOpenISR(config);
#endif
#if (HAL_UART_SPI == 1)
  if (port == HAL_UART_PORT_0)  HalUARTOpenSPI(config);
#endif
#if (HAL_UART_SPI == 2)
  if (port == HAL_UART_PORT_1)  HalUARTOpenSPI(config);
#endif
#if (HAL_UART_USB)
  HalUARTOpenUSB(config);
#endif
#if (HAL_UART_DMA == 0) && (HAL_UART_ISR == 0) && (HAL_UART_SPI == 0)
  // UART is not enabled. Do nothing.
  (void) port;   // unused argument
  (void) config; // unused argument
#endif

  return HAL_UART_SUCCESS;
}

/*****************************************************************************
 * @fn      HalUARTRead
 *
 * @brief   Read a buffer from the UART
 *
 * @param   port - USART module designation
 *          buf  - valid data buffer at least 'len' bytes in size
 *          len  - max length number of bytes to copy to 'buf'
 *
 * @return  length of buffer that was read
 *****************************************************************************/
uint16 HalUARTRead(uint8 port, uint8 *buf, uint16 len)
{
#if (HAL_UART_DMA == 1)
  if (port == HAL_UART_PORT_0)  return HalUARTReadDMA(buf, len);
#endif
#if (HAL_UART_DMA == 2)
  if (port == HAL_UART_PORT_1)  return HalUARTReadDMA(buf, len);
#endif
#if (HAL_UART_ISR == 1)
  if (port == HAL_UART_PORT_0)  return HalUARTReadISR(buf, len);
#endif
#if (HAL_UART_ISR == 2)
  if (port == HAL_UART_PORT_1)  return HalUARTReadISR(buf, len);
#endif
#if (HAL_UART_SPI == 1)
  if (port == HAL_UART_PORT_0)  return HalUARTReadSPI(buf, len);
#endif
#if (HAL_UART_SPI == 2)
  if (port == HAL_UART_PORT_1)  return HalUARTReadSPI(buf, len);
#endif

#if HAL_UART_USB
  return HalUARTRx(buf, len);
#else
  #if (HAL_UART_DMA == 0) && (HAL_UART_ISR == 0) && (HAL_UART_SPI == 0)
    // UART is not enabled. Do nothing.
    (void) port;   // unused argument
    (void) buf;   // unused argument
    (void) len;   // unused argument
  #endif
  return 0;
#endif
}

/******************************************************************************
 * @fn      HalUARTWrite
 *
 * @brief   Write a buffer to the UART.
 *
 * @param   port - UART port
 *          buf  - pointer to the buffer that will be written, not freed
 *          len  - length of
 *
 * @return  length of the buffer that was sent
 *****************************************************************************/
uint16 HalUARTWrite(uint8 port, uint8 *buf, uint16 len)
{
#if (HAL_UART_DMA == 1)
  if (port == HAL_UART_PORT_0)  return HalUARTWriteDMA(buf, len);
#endif
#if (HAL_UART_DMA == 2)
  if (port == HAL_UART_PORT_1)  return HalUARTWriteDMA(buf, len);
#endif
#if (HAL_UART_ISR == 1)
  if (port == HAL_UART_PORT_0)  return HalUARTWriteISR(buf, len);
#endif
#if (HAL_UART_ISR == 2)
  if (port == HAL_UART_PORT_1)  return HalUARTWriteISR(buf, len);
#endif
#if (HAL_UART_SPI == 1)
  if (port == HAL_UART_PORT_0)  return HalUARTWriteSPI(buf, len);
#endif
#if (HAL_UART_SPI == 2)
  if (port == HAL_UART_PORT_1)  return HalUARTWriteSPI(buf, len);
#endif

#if HAL_UART_USB
  HalUARTTx(buf, len);
  return len;
#else
  #if (HAL_UART_DMA == 0) && (HAL_UART_ISR == 0) && (HAL_UART_SPI == 0)
    // UART is not enabled. Do nothing.
    (void) port;   // unused argument
    (void) buf;   // unused argument
    (void) len;   // unused argument
  #endif
  return 0;
#endif
}

/******************************************************************************
 * @fn      HalUARTSuspend
 *
 * @brief   Suspend UART hardware before entering PM mode 1, 2 or 3.
 *
 * @param   None
 *
 * @return  None
 *****************************************************************************/
void HalUARTSuspend( void )
{
#if HAL_UART_ISR
  HalUARTSuspendISR();
#endif
}

/******************************************************************************
 * @fn      HalUARTResume
 *
 * @brief   Resume UART hardware after exiting PM mode 1, 2 or 3.
 *
 * @param   None
 *
 * @return  None
 *****************************************************************************/
void HalUARTResume( void )
{
#if HAL_UART_ISR
  HalUARTResumeISR();
#endif
}

/***************************************************************************************************
 * @fn      HalUARTPoll
 *
 * @brief   Poll the UART.
 *
 * @param   none
 *
 * @return  none
 *****************************************************************************/
void HalUARTPoll(void)
{
#if (HAL_UART_DMA && HAL_UART_SPI)  // When both are defined, port is run-time choice.
  if (HAL_UART_PORT)
  {
    HalUARTPollSPI();
  }
  else
  {
    HalUARTPollDMA();
  }
#else
#if HAL_UART_DMA
  HalUARTPollDMA();
#endif
#if HAL_UART_ISR
  HalUARTPollISR();
#endif
#if HAL_UART_SPI
  HalUARTPollSPI();
#endif
#if HAL_UART_USB
  HalUARTPollUSB();
#endif
#endif
}

/**************************************************************************************************
 * @fn      Hal_UART_RxBufLen()
 *
 * @brief   Calculate Rx Buffer length - the number of bytes in the buffer.
 *
 * @param   port - UART port
 *
 * @return  length of current Rx Buffer
 **************************************************************************************************/
uint16 Hal_UART_RxBufLen( uint8 port )
{
#if (HAL_UART_DMA == 1)
  if (port == HAL_UART_PORT_0)  return HalUARTRxAvailDMA();
#endif
#if (HAL_UART_DMA == 2)
  if (port == HAL_UART_PORT_1)  return HalUARTRxAvailDMA();
#endif
#if (HAL_UART_ISR == 1)
  if (port == HAL_UART_PORT_0)  return HalUARTRxAvailISR();
#endif
#if (HAL_UART_ISR == 2)
  if (port == HAL_UART_PORT_1)  return HalUARTRxAvailISR();
#endif
#if (HAL_UART_SPI == 1)
  if (port == HAL_UART_PORT_0)  return HalUARTRxAvailSPI();
#endif
#if (HAL_UART_SPI == 2)
  if (port == HAL_UART_PORT_1)  return HalUARTRxAvailSPI();
#endif

#if HAL_UART_USB
  return HalUARTRxAvailUSB();
#else
  #if (HAL_UART_DMA == 0) && (HAL_UART_ISR == 0) && (HAL_UART_SPI == 0)
    // UART is not enabled. Do nothing.
    (void) port;   // unused argument
  #endif
  return 0;
#endif
}

void HalUARTIsrDMA(void)
{
#if (HAL_UART_DMA && HAL_UART_SPI)  // When both are defined, port is run-time choice.
  if (HAL_UART_PORT)
  {
    HalUART_DMAIsrSPI();
  }
  else
  {
    HalUART_DMAIsrDMA();
  }
#elif HAL_UART_DMA
  HalUART_DMAIsrDMA();
#elif HAL_UART_SPI
  HalUART_DMAIsrSPI();
#endif
}

#include "ioCC2541.h"

void InitUart1(void)
{
 /*CLKCONCMD &= ~0x40; // ÉèÖÃÏµÍ³Ê±ÖÓÔ´Îª 32MHZ¾§Õñ
 while(CLKCONSTA & 0x40); // µÈ´ý¾§ÕñÎÈ¶¨ 
CLKCONCMD &= ~0x47; // ÉèÖÃÏµÍ³Ö÷Ê±ÖÓÆµÂÊÎª 32MHZ
 */
PERCFG |= 0x02;     // Set UART1 I/O to Alt. 2 location on P1.
P1SEL  |= 0xC0;         // Enable Peripheral control of Rx/Tx on Px.
U1UCR |= 0X02;                 // Flush it.
 
U1CSR |= 0xC0; 
U1GCR |= 11;
U1BAUD |= 216;
UTX1IF = 0; 
IEN0 |= 0x08; // ¿ª×ÜÖÐ¶Ï£¬½ÓÊÕÖÐ¶Ï 
}

/**************************************************************** 
´®¿Ú·¢ËÍ×Ö·û´®º¯Êý 
****************************************************************/ 
void Uart1_Send_Byte(char *Data,int len) 
{
  int j; 
  GPIO_XBEE_DIR_TURN_HIGH();
  HAL_GPIO_CHANGE_DELAY();
  for(j=0;j<len;j++) 
  { 
    U1DBUF = *Data++;
    //U1DBUF = *(Data+j); 
    while(UTX1IF == 0); //·¢ËÍÍê³É±êÖ¾Î»
    UTX1IF = 0; 
  } 
  HAL_GPIO_CHANGE_DELAY();
  GPIO_XBEE_DIR_TURN_LOW();
}

HMC5983DataType hmc5983Data;
HMC5983DataType hmc5983DataStandard;

static  uint8 temp_uart[10] = {0};
HAL_ISR_FUNCTION(port1Isr, URX1_VECTOR)
{ 
  static uint8 index = 0;
  uint8 add_sum,check;
  
  HAL_ENTER_ISR();
  URX1IF = 0; // ÇåÖÐ¶Ï±êÖ¾ 
#if 0
   if(index > 9)
   {
     index = 0;
     while(1);
   }
   else
   {
     temp_uart[index++] = U1DBUF;
   }
#endif
#if 1
  if(U1DBUF == 0xAA)
  {
    temp_uart[0] = U1DBUF; 
    index = 1;
  }
  else if(U1DBUF == 0xBB)
  {
    temp_uart[index] = U1DBUF;
    index = 2;
  }
  else if(U1DBUF == 0xCC)
  {
    temp_uart[index] = U1DBUF;
    index = 3;
  }
  else if(index >2)
  {
    temp_uart[index] = U1DBUF;
    index++;
  }
  
  if( index >= 10 )
  {
    index = 0;
    add_sum = 0;
    for(int i=0;i<9;i++)
        add_sum += temp_uart[i];
    check = 0xFF - add_sum;
    if(temp_uart[0] == 0xAA && temp_uart[1] == 0xBB && temp_uart[2] == 0xCC && temp_uart[9] == check)
    {
        hmc5983Data.x = (unsigned short)temp_uart[3] << 8 | temp_uart[4];
        hmc5983Data.y = (unsigned short)temp_uart[5] << 8 | temp_uart[6];
        hmc5983Data.z = (unsigned short)temp_uart[7] << 8 | temp_uart[8];
        hmc5983Data.state = 0x88;       
        if(SenFlag == 0x88)
        {
          SenFlag = 1;
          hmc5983DataStandard.x = hmc5983Data.x;
          hmc5983DataStandard.y = hmc5983Data.y;
          hmc5983DataStandard.z = hmc5983Data.z;
        }
    }
  }
#endif
  HAL_EXIT_ISR();
}
/******************************************************************************
******************************************************************************/
