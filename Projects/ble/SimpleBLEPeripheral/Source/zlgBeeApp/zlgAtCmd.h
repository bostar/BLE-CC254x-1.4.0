#ifndef __ZLG_AT_CMD_H__
#define __ZLG_AT_CMD_H__

#include "hal_types.h"

typedef enum {
    show_enable = 0x01,
    show_disable = 0x00
}showSrcAddr_cmd_t;

typedef enum {
    unicast = 0x00,
    broadcast = 0x01
}cast_mode_t;

enum nvparamoptcmd{
    enReadLoacalCfg = 0xd1,
    enSetChannelNv = 0xd2,
    enSearchNode = 0xd4,
    enGetRemoteInfo = 0xd5,
    enModifyCfg = 0xd6,
    enResetNode = 0xd9,
    enResetCfg = 0xda
};


enum temporaryparamoptcmd{
    enSetChannel = 0xd1,
    enSetDestAddr = 0xd2,
    enShowSrcAddr = 0xd3,
    enSetIoDirection = 0xd4,
    enReadIoStatus = 0xd5,
    enSetIoStatus = 0xd6,
    enReadAdcValue = 0xd7,
    enEnterSleepMode = 0xd8,
    enSetUnicastOrBroadcast = 0xd9,
    enReadNodeRssi = 0xda
};

typedef struct _dev_info_t{
    unsigned char devName[16];
    unsigned char devPwd[16];
    unsigned char devMode;
    unsigned char devChannel;
    unsigned char devPanid[2];
    unsigned char devLoacalNetAddr[2];
    unsigned char devLoacalIEEEAddr[8];
    unsigned char devDestNetAddr[2];
    unsigned char devDestIEEEAddr[8];
    unsigned char devReserve1;
    unsigned char devPowerLevel;
    unsigned char devRetryNum;
    unsigned char devTranTimeout;       // *10ms
    unsigned char devSerialRate;
    unsigned char devSerialDataB;
    unsigned char devSerialStopB;
    unsigned char devSerialParityB;
    unsigned char devReserve2;
}dev_info_t;

typedef struct _search_info_t {
    unsigned char  deviceType[2];
    unsigned char  channel;
    unsigned char  rate;
    unsigned char  netNum[2];
    unsigned char  localAddr[2];
    unsigned char  runStatus;
}search_info_t;

typedef struct _uartReturnStatus_t {
    unsigned char readLocalCfg_SUCCESS;
    unsigned char writeLocalCfg_SUCCESS;
    unsigned char applyNetWork_SUCCESS;
    unsigned char gpioDirectionSet_SUCCESS;
    unsigned char restoreSuccessFlag;
    unsigned char readAdcSuccessFlag;
    unsigned char ledBitState;
    unsigned short adc_value;
}uartReturnStatus_t;

typedef struct _zlgContext_t {
  search_info_t stDevInfo;
  uartReturnStatus_t uartReturnFlags;
}zlgContext_t;

extern uartReturnStatus_t uartReturnFlag;
extern dev_info_t * const stDevInfo;
extern unsigned short localAddress;
extern unsigned char zlgIOLevel;

void read_local_cfg(void);
void search_node(void);
void get_remote_info(uint16 DstAddr);
void write_remote_cfg(uint16 DstAddr, dev_info_t *DevInfo );
void write_local_cfg(uint16 LocAddr, dev_info_t *DevInfo );
void reset_node(uint16 DstAddr);
void restore_factory_settings(uint16 DstAddr);

void send_data_to_remote_node(uint16 destAddr,unsigned char *data,int len);
//void send_data_to_remote_node(unsigned char *destAddr,unsigned char *data,int len);
uint8 receive_data( uint8 *buf, uint16 len );
void set_temporary_channel(uint8 channel);
void set_temporary_DestAddr(uint16 DestAddr);
void set_temporary_ShowSrcAddr(showSrcAddr_cmd_t cmd);
void set_temporary_io_dir(uint16 DstAddr,uint8 IO_Dir);
void read_temporary_io_level(uint16 DstAddr);
void set_temporary_io_level(uint16 DstAddr,uint8 IO_Level);
void read_temporary_adc_value(uint16 DstAddr);
void temporary_enter_sleep_mode(void);
void set_temporary_cast_mode(cast_mode_t mode);
void read_temporary_node_rssi(uint16 DstAddr);
void send_data_to_local_node(uint16 DstAddr,uint8 *data,uint8 len);

#endif
