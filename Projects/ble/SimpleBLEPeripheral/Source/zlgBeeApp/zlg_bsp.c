#include "zlg_bsp.h"
#include "zlgAtCmd.h"
#include "hal_zlg.h"

#if defined HAL_ZLG_ZIGBEE
void initZlgBsp(void)
{
    set_temporary_io_dir(localAddress,ZLG_GPIO_DIR);
}

void setBeepOn(void)
{
    set_temporary_io_level(localAddress,BEEPON);
}

void setBeepOff(void)
{
    //zlgIOLevel &= ~BEEPON;
    set_temporary_io_level(localAddress,BEEPOFF);
}

void setLedBit(unsigned char bits)
{
    set_temporary_io_level(localAddress,bits);
}
 
void setMotorStop(void)
{
//    HalGpioSet( HAL_GPIO_ZM516X_MOTOR1, 0 );
//    HalGpioSet( HAL_GPIO_ZM516X_MOTOR2, 0 );
    GPIO_ZM516X_MOTOR1_TURN_LOW();
    GPIO_ZM516X_MOTOR2_TURN_LOW();
}

void setMotorForward(void)
{
    HalGpioSet( HAL_GPIO_ZM516X_MOTOR1, 0 );
    HalGpioSet( HAL_GPIO_ZM516X_MOTOR2, 1 );
}

void setMotorReverse(void)
{
    HalGpioSet( HAL_GPIO_ZM516X_MOTOR1, 1 );
    HalGpioSet( HAL_GPIO_ZM516X_MOTOR2, 0 );
}

#endif