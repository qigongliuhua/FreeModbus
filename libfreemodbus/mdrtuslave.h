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

    mdVOID (*mdRTUSendString)(ModbusRTUSlaveHandler handler, mdU8* data, mdU32 length);
    mdVOID (*mdRTUHandleCode1)(ModbusRTUSlaveHandler handler);
    mdVOID (*mdRTUHandleCode2)(ModbusRTUSlaveHandler handler);
    mdVOID (*mdRTUHandleCode3)(ModbusRTUSlaveHandler handler);
    mdVOID (*mdRTUHandleCode4)(ModbusRTUSlaveHandler handler);
    mdVOID (*mdRTUHandleCode5)(ModbusRTUSlaveHandler handler);
    mdVOID (*mdRTUHandleCode6)(ModbusRTUSlaveHandler handler);
    mdVOID (*mdRTUHandleCode15)(ModbusRTUSlaveHandler handler);
    mdVOID (*mdRTUHandleCode16)(ModbusRTUSlaveHandler handler);
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