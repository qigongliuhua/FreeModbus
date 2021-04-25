
#include <stdlib.h>
#include <string.h>
#include "mdrtuslave.h"
#include "mdcrc16.h"



#define LOW(n) ((mdU16)n%256)
#define HIGH(n) ((mdU16)n/256)
#define ToU16(high,low) ((((mdU16)high & 0x00ff)<<8) | \
                            ((mdU16)low & 0x00ff))


#define ERROR1  1   //接收帧时错误
#define ERROR2  2   //帧长度错误
#define ERROR3  3   //CRC校验错误
#define ERROR4  4   //站号错误
#define ERROR5  5   //未知的功能码

static mdVOID mdRTUError(ModbusRTUSlaveHandler handler, mdU8 error)
{

}


/* ================================================================== */
/*                        接口区                                       */
/* ================================================================== */
/*
    portRtuPushChar
        @handler 句柄
        @c 待发送字符
        @return
    接口：接收一个字符
*/
static mdVOID portRtuPushChar(ModbusRTUSlaveHandler handler,mdU8 c){
    ReceiveBufferHandle recbuf = handler->receiveBuffer;
    recbuf->buf[recbuf->count++] = c;
}


#define TIMER_CLEAN() do{\
                lastCount = 0;\
                ustime = 0;\
                timeSum = 0;\
                error = 0;}while(0)
/*
    mdRtuBaseTimerTick
        @handler 句柄
        @time   时长跨度，单位 us
        @return
    接口：帧间隙监控
*/
static mdVOID portRtuTimerTick(ModbusRTUSlaveHandler handler, mdU32 ustime)
{
    static mdU32 lastCount;
    static mdU64 timeSum;
    static mdFMStatus error;
    if (handler->receiveBuffer->count > 0)
    {
        if (handler->receiveBuffer->count != lastCount)
        {
            if (timeSum > handler->invalidTime)
            {
                error++;
            }
            lastCount = handler->receiveBuffer->count;
            timeSum = 0;
        }
        if(timeSum > handler->stopTime)
        {
            if(error == 0 || IGNORE_LOSS_FRAME != 0)
            {
                handler->mdRTUCenterProcessor(handler);
            }
            else
            {
                handler->mdRTUError(handler, ERROR1);
            }
            mdClearReceiveBuffer(handler->receiveBuffer);
            TIMER_CLEAN();
        }
        timeSum += ustime;
    }
    else
    {
        TIMER_CLEAN();
    }

}


/* ================================================================== */
/*                        core                                        */
/* ================================================================== */
#define MODBUS_CODE_1 1
#define MODBUS_CODE_2 2
#define MODBUS_CODE_3 3
#define MODBUS_CODE_4 4
#define MODBUS_CODE_5 5
#define MODBUS_CODE_6 6
#define MODBUS_CODE_15 15
#define MODBUS_CODE_16 16

#define mdGetSlaveId()          (recbuf[0])
#define mdGetCrc16()            (ToU16(recbuf[reclen-2],recbuf[reclen-1]))
#define mdGetCode()             (recbuf[1])
#define mdmalloc(pointer, type, length) pointer = (type*)malloc(sizeof(type) * length);\
                                        memset(pointer, 0, sizeof(type) * length)

static mdVOID mdRTUSendString(ModbusRTUSlaveHandler handler, mdU8* data, mdU32 length)
{
    for (mdU32 i = 0; i < length; i++)
    {
        handler->mdRTUPopChar(handler, data[i]);
    }
}

static mdVOID mdRTUHandleCode1(ModbusRTUSlaveHandler handler)
{
    mdU32 reclen = handler->receiveBuffer->count;
    mdU8* recbuf = handler->receiveBuffer->buf;
    RegisterPoolHandle regPool = handler->registerPool;
    mdU16 startAddress = ToU16(recbuf[2], recbuf[3]);
    mdBit* data;
    mdU16 length = ToU16(recbuf[4], recbuf[5]);
    mdU8 *data2;
    mdU8 length2 = 0;
    mdU16 crc;

    mdmalloc(data, mdBit, length);
    regPool->mdReadCoils(regPool, startAddress, length, data);
    length2 = length % 8 > 0 ? length / 8 + 1: length / 8;
    mdmalloc(data2, mdU8, 5 + length2);
    data2[0] = recbuf[0];
    data2[1] = recbuf[1];
    data2[2] = length2;
    for (mdU32 i = 0; i <  length2; i++)
    {
        for (mdU32 j = 0; j < 8 && (i * 8 + j) < length; j++)
        {
            data2[3 + i] |= ((data[i * 8 + j] & 0x01) << j);
        }
    }
    crc = mdCrc16(data2, 3 + length2);
    data2[3 + length2] = HIGH(crc);
    data2[4 + length2] = LOW(crc);
    handler->mdRTUSendString(handler, data2, 5 + length2);
    free(data);
    free(data2);
}

static mdVOID mdRTUHandleCode2(ModbusRTUSlaveHandler handler)
{
    mdU32 reclen = handler->receiveBuffer->count;
    mdU8* recbuf = handler->receiveBuffer->buf;
    RegisterPoolHandle regPool = handler->registerPool;
    mdU16 startAddress = ToU16(recbuf[2], recbuf[3]);
    mdBit* data;
    mdU16 length = ToU16(recbuf[4], recbuf[5]);
    mdU8 *data2;
    mdU8 length2 = 0;
    mdU16 crc;

    mdmalloc(data, mdBit, length);
    regPool->mdReadInputCoils(regPool, startAddress, length, data);
    length2 = length % 8 > 0 ? length / 8 + 1: length / 8;
    mdmalloc(data2, mdU8, 5 + length2);
    data2[0] = recbuf[0];
    data2[1] = recbuf[1];
    data2[2] = length2;
    for (mdU32 i = 0; i <  length2; i++)
    {
        for (mdU32 j = 0; j < 8 && (i * 8 + j) < length; j++)
        {
            data2[3 + i] |= ((data[i * 8 + j] & 0x01) << j);
        }
    }
    crc = mdCrc16(data2, 3 + length2);
    data2[3 + length2] = HIGH(crc);
    data2[4 + length2] = LOW(crc);
    handler->mdRTUSendString(handler, data2, 5 + length2);
    free(data);
    free(data2);
}

static mdVOID mdRTUHandleCode3(ModbusRTUSlaveHandler handler)
{
    mdU32 reclen = handler->receiveBuffer->count;
    mdU8* recbuf = handler->receiveBuffer->buf;
    RegisterPoolHandle regPool = handler->registerPool;
    mdU16 startAddress = ToU16(recbuf[2], recbuf[3]);
    mdU16* data;
    mdU16 length = ToU16(recbuf[4], recbuf[5]);
    mdU8 *data2;
    mdU16 crc;

    mdmalloc(data, mdU16, length);
    regPool->mdReadHoldRegisters(regPool, startAddress, length, data);
    mdmalloc(data2, mdU8, 5 + length * 2);
    data2[0] = recbuf[0];
    data2[1] = recbuf[1];
    data2[2] = (mdU8)(length * 2);
    for (mdU32 i = 0; i <  length; i++)
    {
        data2[3 + 2 * i] = HIGH(data[i]);
        data2[3 + 2 * i + 1] = LOW(data[i]);
    }
    crc = mdCrc16(data2, 3 + length * 2);
    data2[3 + length * 2] = HIGH(crc);
    data2[4 + length * 2] = LOW(crc);
    handler->mdRTUSendString(handler, data2, 5 + length * 2);
    free(data);
    free(data2);
}

static mdVOID mdRTUHandleCode4(ModbusRTUSlaveHandler handler)
{
    mdU32 reclen = handler->receiveBuffer->count;
    mdU8* recbuf = handler->receiveBuffer->buf;
    RegisterPoolHandle regPool = handler->registerPool;
    mdU16 startAddress = ToU16(recbuf[2], recbuf[3]);
    mdU16* data;
    mdU16 length = ToU16(recbuf[4], recbuf[5]);
    mdU8 *data2;
    mdU16 crc;

    mdmalloc(data, mdU16, length);
    regPool->mdReadInputRegisters(regPool, startAddress, length, data);
    mdmalloc(data2, mdU8, 5 + length * 2);
    data2[0] = recbuf[0];
    data2[1] = recbuf[1];
    data2[2] = (mdU8)(length * 2);
    for (mdU32 i = 0; i <  length; i++)
    {
        data2[3 + 2 * i] = HIGH(data[i]);
        data2[3 + 2 * i + 1] = LOW(data[i]);
    }
    crc = mdCrc16(data2, 3 + length * 2);
    data2[3 + length * 2] = HIGH(crc);
    data2[4 + length * 2] = LOW(crc);
    handler->mdRTUSendString(handler, data2, 5 + length * 2);
    free(data);
    free(data2);
}

static mdVOID mdRTUHandleCode5(ModbusRTUSlaveHandler handler)
{
    mdU32 reclen = handler->receiveBuffer->count;
    mdU8* recbuf = handler->receiveBuffer->buf;
    RegisterPoolHandle regPool = handler->registerPool;
    mdU16 startAddress = ToU16(recbuf[2], recbuf[3]);
    mdBit data = ToU16(recbuf[4], recbuf[5]) > 0 ? mdHigh : mdLow;
    regPool->mdWriteCoil(regPool, startAddress, data);
    handler->mdRTUSendString(handler, recbuf, reclen);
}

static mdVOID mdRTUHandleCode6(ModbusRTUSlaveHandler handler)
{
    mdU32 reclen = handler->receiveBuffer->count;
    mdU8* recbuf = handler->receiveBuffer->buf;
    RegisterPoolHandle regPool = handler->registerPool;
    mdU16 startAddress = ToU16(recbuf[2], recbuf[3]);
    mdU16 data = ToU16(recbuf[4], recbuf[5]);
    regPool->mdWriteHoldRegister(regPool, startAddress, data);
    handler->mdRTUSendString(handler, recbuf, reclen);
}

static mdVOID mdRTUHandleCode15(ModbusRTUSlaveHandler handler)
{
    mdU32 reclen = handler->receiveBuffer->count;
    mdU8* recbuf = handler->receiveBuffer->buf;
    RegisterPoolHandle regPool = handler->registerPool;
    mdU16 startAddress = ToU16(recbuf[2], recbuf[3]);
    mdU16 length = ToU16(recbuf[4], recbuf[5]);
    mdU8 byteLength = recbuf[6];
    mdU8 *data;
    mdU16 crc;
    for (mdU32 i = 0; i < length; i++)
    {
        regPool->mdWriteCoil(regPool, startAddress + i, ((recbuf[7 + i / 8] >> (i % 8)) & 0x01));
    }
    mdmalloc(data, mdU8, 8);
    memcpy(data, recbuf, 6);
    crc = mdCrc16(data, 6);
    data[6] = HIGH(crc);
    data[7] = LOW(crc);
    handler->mdRTUSendString(handler, data, 8);
    free(data);
}

static mdVOID mdRTUHandleCode16(ModbusRTUSlaveHandler handler)
{
    mdU32 reclen = handler->receiveBuffer->count;
    mdU8* recbuf = handler->receiveBuffer->buf;
    RegisterPoolHandle regPool = handler->registerPool;
    mdU16 startAddress = ToU16(recbuf[2], recbuf[3]);
    mdU16 length = ToU16(recbuf[4], recbuf[5]);
    mdU8 byteLength = recbuf[6];
    mdU8 *data;
    mdU16 crc;
    for (mdU32 i = 0; i < length; i++)
    {
        regPool->mdWriteHoldRegister(regPool, startAddress + i,
             ToU16(recbuf[7 + 2 * i], recbuf[7 + 2 * i + 1]));
    }
    mdmalloc(data, mdU8, 8);
    memcpy(data, recbuf, 6);
    crc = mdCrc16(data, 6);
    data[6] = HIGH(crc);
    data[7] = LOW(crc);
    handler->mdRTUSendString(handler, data, 8);
    free(data);
}
/*
    mdModbusRTUCenterProcessor
        @handler 句柄
        @receFrame 待处理的帧（已校验通过）
    处理一帧，并且通过接口发送处理结果
*/
static mdVOID mdRTUCenterProcessor(ModbusRTUSlaveHandler handler)
{
    mdU32 reclen = handler->receiveBuffer->count;
    mdU8* recbuf = handler->receiveBuffer->buf;
    if (reclen < 3)
    {
        handler->mdRTUError(handler, ERROR2);
        return;
    }
    if(mdCrc16(recbuf,reclen-2) != mdGetCrc16()
        && CRC_CHECK != 0)
    {
        handler->mdRTUError(handler, ERROR3);
        return;
    }
    if (mdGetSlaveId() != handler->slaveId)
    {
        handler->mdRTUError(handler, ERROR4);
        return;
    }
    switch (mdGetCode())
    {
    case MODBUS_CODE_1:
        handler->mdRTUHandleCode1(handler);
        break;
    case MODBUS_CODE_2:
        handler->mdRTUHandleCode2(handler);
        break;
    case MODBUS_CODE_3:
        handler->mdRTUHandleCode3(handler);
        break;
    case MODBUS_CODE_4:
        handler->mdRTUHandleCode4(handler);
        break;
    case MODBUS_CODE_5:
        handler->mdRTUHandleCode5(handler);
        break;
    case MODBUS_CODE_6:
        handler->mdRTUHandleCode6(handler);
        break;
    case MODBUS_CODE_15:
        handler->mdRTUHandleCode15(handler);
        break;
    case MODBUS_CODE_16:
        handler->mdRTUHandleCode16(handler);
        break;
    default:
        handler->mdRTUError(handler, ERROR5);
        break;
    }
}

/* ================================================================== */
/*                        API                                         */
/* ================================================================== */
/*
    mdCreateModbusRTUSlave
        @handler 句柄
        @mdRtuPopChar 字符发送函数
    创建一个modbus从机
*/
mdSTATUS mdCreateModbusRTUSlave(ModbusRTUSlaveHandler *handler, struct ModbusRTUSlaveRegisterInfo info)
{
    (*handler) = (ModbusRTUSlaveHandler)malloc(sizeof(struct ModbusRTUSlave));
    if ((*handler) != NULL)
    {
        (*handler)->mdRTUPopChar = info.mdRTUPopChar;
        (*handler)->mdRTUCenterProcessor = mdRTUCenterProcessor;
        (*handler)->mdRTUError = mdRTUError;
        (*handler)->slaveId = info.slaveId;
        (*handler)->invalidTime = (int)(1.5 * 8 * 1000 * 1000 / info.usartBaudRate);
        (*handler)->stopTime = (int)(3.5 * 8 * 1000 * 1000 / info.usartBaudRate);
        (*handler)->portRTUPushChar = portRtuPushChar;
        (*handler)->portRTUTimerTick = portRtuTimerTick;
        (*handler)->mdRTUSendString = mdRTUSendString;
        (*handler)->mdRTUHandleCode1 = mdRTUHandleCode1;
        (*handler)->mdRTUHandleCode2 = mdRTUHandleCode2;
        (*handler)->mdRTUHandleCode3 = mdRTUHandleCode3;
        (*handler)->mdRTUHandleCode4 = mdRTUHandleCode4;
        (*handler)->mdRTUHandleCode5 = mdRTUHandleCode5;
        (*handler)->mdRTUHandleCode6 = mdRTUHandleCode6;
        (*handler)->mdRTUHandleCode15 = mdRTUHandleCode15;
        (*handler)->mdRTUHandleCode16 = mdRTUHandleCode16;

        if(mdCreateRegisterPool(&((*handler)->registerPool)) &&
            mdCreateReceiveBuffer(&((*handler)->receiveBuffer))){
                return mdTRUE;
        }
        else
        {
            free((*handler));
        }
    }
    return mdFALSE;
}


/*
    mdDestoryModbusRTUSlave
        @handler 句柄
    销毁一个modbus从机
*/
mdVOID mdDestoryModbusRTUSlave(ModbusRTUSlaveHandler *handler){
    mdDestoryRegisterPool(&((*handler)->registerPool));
    mdDestoryReceiveBuffer(&((*handler)->receiveBuffer));
    free(*handler);
    (*handler) = NULL;
}