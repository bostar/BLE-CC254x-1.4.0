#ifndef __ZLG_PROTOCOL_H__
#define __ZLG_PROTOCOL_H__

typedef enum _parkingEvent_t {
    cmdVehicleComming = 0x00,
    cmdVehicleLeave = 0x01,
    cmdLockSuccess = 0x02,
    cmdLockFailed = 0x03,
    cmdUnlockSuccess = 0x04,
    cmdUnlockFailed = 0x05,
    cmdLockingOrUnlocking = 0x06,
} parkingEvent_t;

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
    cmdDataRequest = 0x0A
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
void eventReport(parkingEvent_t event);
void batteryRemainingReport(unsigned char voltage);
#endif
