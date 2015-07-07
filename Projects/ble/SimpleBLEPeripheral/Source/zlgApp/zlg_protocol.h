#ifndef __ZLG_PROTOCOL_H__
#define __ZLG_PROTOCOL_H__

typedef enum _cmdLockControl {
    UNLOCK = 0x00,
    LOCK = 0x01
}LockSW_t;

enum cfg_cmd {
	cmdCheckIn = 0x00,
	cmdAckCheckIn = 0x01,
	cmdChangeNodeType = 0x02,
	cmdAckChangeNodeType = 0x03,
	cmdChangePanidChannel = 0x04,
	cmdAllNodeReset = 0x05,
	cmdLinkTest = 0x06,
	cmdAckLinkTest = 0x07
};

enum sen_cmd {
	cmdSensorCal = 0x00,
};

enum tst_cmd {
	cmdBeepTest = 0x00,
	cmdLedTest = 0x01,
	cmdMotorTest = 0x02
};

enum beep_cmd {
	cmdSilence = 0x00,
	cmdBuzz = 0x01
};

enum motor_cmd {
	cmdStop = 0x00,
	cmdForward = 0x01,
	cmdReverse = 0x02
};

void applicateForNetwork(unsigned char *IEEEAddress);
void ackChangeNodeType(void);
void ackLinkTest(void);
void switchLockControl(LockSW_t cmd);

#endif
