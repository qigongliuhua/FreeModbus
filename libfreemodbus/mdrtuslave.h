#ifndef __MDRTUSLAVE_H__
#define __MDRTUSLAVE_H__


#include "mdtype.h"
#include "mdconfig.h"
#include "mdregpool.h"
#include "mdrecbuffer.h"



typedef struct ModbusRTUSlave* ModbusRTUSlaveHandle;

struct ModbusRTUSlave{
    mdU8 slaveId;
    mdU32 usartBaudRate;
    mdU32 stopTime,invalidTime;
    ReceiveBufferHandle receiveBuffer;
    RegisterPoolHandle registerPool;
    mdVOID (*mdRTUPopChar)(ModbusRTUSlaveHandle handle, mdU8 c);
    mdVOID (*mdRTUCenterProcessor)(ModbusRTUSlaveHandle handle);

    mdVOID (*portRTUPushChar)(ModbusRTUSlaveHandle handle,mdU8 c);
    mdVOID (*portRTUTimerTick)(ModbusRTUSlaveHandle handle, mdU32 ustime);
};


struct ModbusRTUSlaveRegisterInfo
{
    mdU8 slaveId;
    mdU32 usartBaudRate;
    mdVOID (*mdRTUPopChar)(ModbusRTUSlaveHandle handle, mdU8 c);
};

mdAPI mdSTATUS mdCreateModbusRTUSlave(ModbusRTUSlaveHandle *handle,struct ModbusRTUSlaveRegisterInfo info);
mdAPI mdVOID mdDestoryModbusRTUSlave(ModbusRTUSlaveHandle *handle);

#endif