#ifndef _XBEE_HAL_H_
#define _XBEE_HAL_H_

typedef enum
{
    GPIO_XBEE_SLEEP_INDER  = 1,
    GPIO_XBEE_RTS          = 2,
    GPIO_XBEE_CTS          = 3,
    GPIO_XBEE_RESET        = 4,
    GPIO_XBEE_SLEEP        = 5,
    GPIO_XBEE_MOTOR1       = 6,
    GPIO_XBEE_MOTOR2       = 7,
    GPIO_XBEE_MO_EN        = 8,
    GPIO_XBEE_KEY1         = 9,
    GPIO_XBEE_KEY2         = 10,
    GPIO_XBEE_KEY3         = 11
}GPIOType;


void HalXbeeInit(void);
uint8 HalGpioSet(uint8 pin, uint8 level);
uint8 HalGpioGet(GPIOType pin);
















#endif