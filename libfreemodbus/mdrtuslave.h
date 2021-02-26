#ifndef __MDRTUSLAVE_H__
#define __MDRTUSLAVE_H__


#include "mdtype.h"
#include "mdconfig.h"
#include "mdregpool.h"
#include "mdrecbuffer.h"



typedef struct ModbusRTUSlave* ModbusRTUSlaveHandler;

struct ModbusRTUSlave{
    mdU8 slaveId;
    mdU32 usartBaudRate;
    mdU32 stopTime,invalidTime;
    ReceiveBufferHandle receiveBuffer;
    RegisterPoolHandle registerPool;
    mdVOID (*mdRTUPopChar)(ModbusRTUSlaveHandler handler, mdU8 c);
    mdVOID (*mdRTUCenterProcessor)(ModbusRTUSlaveHandler handler);
    mdVOID (*mdRTUError)(ModbusRTUSlaveHandler handler, mdU8 error);

    mdVOID (*portRTUPushChar)(ModbusRTUSlaveHandler handler,mdU8 c);
    mdVOID (*portRTUTimerTick)(ModbusRTUSlaveHandler handler, mdU32 ustime);
};


struct ModbusRTUSlaveRegisterInfo
{
    mdU8 slaveId;
    mdU32 usartBaudRate;
    mdVOID (*mdRTUPopChar)(ModbusRTUSlaveHandler handler, mdU8 c);
};

mdAPI mdSTATUS mdCreateModbusRTUSlave(ModbusRTUSlaveHandler *handler,struct ModbusRTUSlaveRegisterInfo info);
mdAPI mdVOID mdDestoryModbusRTUSlave(ModbusRTUSlaveHandler *handler);

#endif