#ifndef __ZLG_BSP_H__
#define __ZLG_BSP_H__

#define ZLG_GPIO_DIR        0x7C
#define BEEPON         (unsigned char)(0x01 << 2)
#define BEEPOFF        (unsigned char)(0x00)


void initZlgBsp(void);
void setBeepOn(void);
void setBeepOff(void);
void setLedBit(unsigned char bits);
void setMotorStop(void);
void setMotorForward(void);
void setMotorReverse(void);

#endif
