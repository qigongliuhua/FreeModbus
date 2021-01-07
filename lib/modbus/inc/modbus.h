#ifndef __MODBUS_H__
#define __MODBUS_H__


#include "mdtype.h"
#include "mdconfig.h"

#include "mdregpool.h"
#include "mdcrc16.h"


typedef mdVOID (*RtuPopCharHandle)(ModbusSlaveHandle handle, mdU8 c);


#define ReceiveStatus_Ready 0
#define ReceiveStatus_Ing   1


typedef struct ReceiveBuffer* ReceiveBufferHandle;
struct ReceiveBuffer
{
    mdU8  buf[MODBUS_PDU_SIZE_MAX];
    mdU32 count;
    mdU64 time;
    mdU8 status;

};


#define FrameStatus_Normal 0
#define FrameStatus_Error 1

typedef struct ModbusRtuFrame* ModbusRtuFrameHandle;
struct ModbusRtuFrame
{
    mdU8 *frame;
    mdU16 len;
    mdFMStatus status;
};
 

typedef struct ModbusSlave* ModbusSlaveHandle;
struct ModbusSlave{
    mdU8 slaveId;
    mdU32 usartBaudRate;
    ReceiveBufferHandle receiveBuffer;
    RegisterPoolHandle registerPool;
    RtuPopCharHandle  mdRtuPopChar;

    mdVOID (*portRtuDriver)(ModbusSlaveHandle handle);
    mdVOID (*portRtuPushChar)(ModbusSlaveHandle handle,mdU8 c);

    mdVOID (*portRtu1000usTimerTick)(ModbusSlaveHandle handle);
    mdVOID (*portRtu100usTimerTick)(ModbusSlaveHandle handle);
    mdVOID (*portRtu10usTimerTick)(ModbusSlaveHandle handle);
    mdVOID (*portRtu1usTimerTick)(ModbusSlaveHandle handle);
};



#define MODBUS_CODE_1 1
#define MODBUS_CODE_2 2
#define MODBUS_CODE_3 3
#define MODBUS_CODE_4 4
#define MODBUS_CODE_5 5
#define MODBUS_CODE_6 6
#define MODBUS_CODE_15 15
#define MODBUS_CODE_16 16



extern mdSTATUS mdCreateModbusSlave(ModbusSlaveHandle *handle, RtuPopCharHandle mdRtuPopChar);
extern mdVOID mdDestoryModbusSlave(ModbusSlaveHandle *handle);

#endif