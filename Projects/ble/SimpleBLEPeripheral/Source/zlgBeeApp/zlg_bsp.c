#include "zlg_bsp.h"
#include "zlgAtCmd.h"
#include "hal_zlg.h"

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