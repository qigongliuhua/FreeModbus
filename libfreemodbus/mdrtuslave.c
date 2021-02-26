
#include <stdlib.h>
#include <string.h>
#include "mdrtuslave.h"
#include "mdcrc16.h"



#define LOW(n) ((mdU16)n%256)
#define HIGH(n) ((mdU16)n/256)
#define ToDouble(n) ((double)n)
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
/*
    mdModbusRTUCenterProcessor
        @handler 句柄
        @receFrame 待处理的帧（已校验通过）
    处理一帧，并且通过接口发送处理结果
*/
static mdVOID mdRTUCenterProcessor(ModbusRTUSlaveHandler handler)
{
#if (DEBUG != 0)
    for (size_t i = 0; i < handler->receiveBuffer->count; i++)
    {
        handler->mdRTUPopChar(handler,handler->receiveBuffer->buf[i]);
    }
    handler->mdRTUPopChar(handler,'\n');
#endif
    mdU32 reclen = handler->receiveBuffer->count;
    mdU8* recbuf = handler->receiveBuffer->buf;
    if (reclen < 3)
    {
        handler->mdRTUError(handler, ERROR2);
        return;
    }
    if(mdCrc16(recbuf,reclen-2) != mdGetCrc16()
        && IGNORE_CRC_CHECK != 0)
    {
        handler->mdRTUError(handler, ERROR3);
        return;
    }
    if (mdGetSlaveId() != handler->slaveId)
    {
        handler->mdRTUError(handler, ERROR4);
        return;
    }

    if(mdGetCode() == MODBUS_CODE_1)
    {

    }
    else if(mdGetCode() == MODBUS_CODE_2)
    {

    }
    else if(mdGetCode() == MODBUS_CODE_3)
    {

    }
    else if(mdGetCode() == MODBUS_CODE_4)
    {

    }
    else if(mdGetCode() == MODBUS_CODE_5)
    {

    }
    else if(mdGetCode() == MODBUS_CODE_6)
    {

    }
    else if(mdGetCode() == MODBUS_CODE_15)
    {

    }
    else if(mdGetCode() == MODBUS_CODE_16)
    {

    }
    else
    {
        handler->mdRTUError(handler, ERROR5);
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