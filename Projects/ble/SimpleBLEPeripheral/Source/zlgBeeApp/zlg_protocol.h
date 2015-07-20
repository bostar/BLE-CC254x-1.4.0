#ifndef __ZLG_PROTOCOL_H__
#define __ZLG_PROTOCOL_H__

typedef enum _cmdLockReturnState {
    cmdLockOrUnlockSUCCESS = 0x01,
    cmdunLockFailed = 0x02,
    cmdLockFailed = 0x03
}lockRS_t;

typedef enum _cmdCarState {
        cmdCarArrive = 0x01,
        cmdCarLeave = 0x02
} carState_t;

enum cfg_cmd {
	cmdCheckIn = 0x00,
	cmdAckCheckIn = 0x01,
//	cmdChangeNodeType = 0x02,
//	cmdAckChangeNodeType = 0x03,
//	cmdChangePanidChannel = 0x04,
//	cmdAllNodeReset = 0x05,
	cmdLinkTest = 0x06,
	cmdAckLinkTest = 0x07,
    cmdRestoreFactoryConfig = 0x08,
    cmdHeartBeatPkg = 0x09,
    cmdDateRequest = 0x0A
};

enum sen_cmd {
	cmdSensorCalibration = 0x00,
    cmdEventReport = 0x01,
    cmdBatteryRemainReport = 0x02
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
void ackLinkTest(unsigned char *IEEEAddress);
void dateRequset(void);
void ackSwitchLockControl(lockRS_t cmd);
void eventReport(carState_t carState);
void batteryRemainingReport(unsigned char voltage);
#endif
