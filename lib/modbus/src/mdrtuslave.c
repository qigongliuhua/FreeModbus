
#include <stdlib.h>
#include <string.h>
#include "mdrtuslave.h"
#include "mdcrc16.h"

#define MODBUS_CODE_1 1
#define MODBUS_CODE_2 2
#define MODBUS_CODE_3 3
#define MODBUS_CODE_4 4
#define MODBUS_CODE_5 5
#define MODBUS_CODE_6 6
#define MODBUS_CODE_15 15
#define MODBUS_CODE_16 16

#define LOW(n) ((mdU16)n%256)
#define HIGH(n) ((mdU16)n/256)
#define ToDouble(n) ((double)n)
#define ToU16(high,low) ((((mdU16)high & 0x00ff)<<8) | \
                            ((mdU16)low & 0x00ff))



/* ================================================================== */
/*                        接口区                                       */
/* ================================================================== */
/*
    portRtuPushChar
        @handle 句柄
        @c 待发送字符
        @return
    接口：接收一个字符
*/
static mdVOID portRtuPushChar(ModbusRTUSlaveHandle handle,mdU8 c){
    ReceiveBufferHandle recbuf = handle->receiveBuffer;
    recbuf->buf[recbuf->count++] = c;
}


#define TIMER_CLEAN() do{\
                lastCount = 0;\
                ustime = 0;\
                error = 0;}while(0)
/*
    mdRtuBaseTimerTick
        @handle 句柄
        @time   时长跨度，单位 us
        @return
    接口：帧间隙监控
*/
static mdVOID portRtuTimerTick(ModbusRTUSlaveHandle handle, mdU32 ustime)
{
    static mdU32 lastCount;
    static mdU64 timeSum;
    static mdFMStatus error;
    if (handle->receiveBuffer->count > 0)
    {
        timeSum += ustime;
        if(timeSum > handle->invalidTime)
        {
            if(handle->receiveBuffer->count != lastCount)
            {
                lastCount = handle->receiveBuffer->count;
                error++;
            }
        }
        if(timeSum > handle->stopTime)
        {
            if(--error == 0)
            {
                handle->mdRTUCenterProcessor(handle);
            }
            mdClearReceiveBuffer(handle->receiveBuffer);
            TIMER_CLEAN();
        }
    }
    else
    {
        TIMER_CLEAN();
    }

}


/* ================================================================== */
/*                        core                                        */
/* ================================================================== */
/*
    mdModbusRTUCenterProcessor
        @handle 句柄
        @receFrame 待处理的帧（已校验通过）
    处理一帧，并且通过接口发送处理结果
*/
static mdVOID mdRTUCenterProcessor(ModbusRTUSlaveHandle handle)
{
    

}


/* ================================================================== */
/*                        API                                         */
/* ================================================================== */
/*
    mdCreateModbusRTUSlave
        @handle 句柄
        @mdRtuPopChar 字符发送函数
    创建一个modbus从机
*/
mdSTATUS mdCreateModbusRTUSlave(ModbusRTUSlaveHandle *handle, struct ModbusRTUSlaveRegisterInfo* info)
{
    (*handle) = (ModbusRTUSlaveHandle)malloc(sizeof(struct ModbusRTUSlave));
    if ((*handle) != NULL)
    {
        (*handle)->mdRTUPopChar = info->mdRTUPopChar;
        (*handle)->mdRTUCenterProcessor = mdRTUCenterProcessor;
        (*handle)->slaveId = info->slaveId;
        (*handle)->invalidTime = (int)(1.5 * 8 * 1000 * 1000 / info->usartBaudRate);
        (*handle)->stopTime = (int)(3.5 * 8 * 1000 * 1000 / info->usartBaudRate);
        (*handle)->portRTUPushChar = portRtuPushChar;
        (*handle)->portRTUTimerTick = portRtuTimerTick;

        if(mdCreateRegisterPool(&((*handle)->registerPool)) &&
            mdCreateReceiveBuffer(&((*handle)->receiveBuffer))){
                return mdTRUE;
        }
        else
        {
            free((*handle));
        }
    }
    return mdFALSE;
}


/*
    mdDestoryModbusRTUSlave
        @handle 句柄
    销毁一个modbus从机
*/
mdVOID mdDestoryModbusRTUSlave(ModbusRTUSlaveHandle *handle){
    mdDestoryRegisterPool(&((*handle)->registerPool));
    mdDestoryReceiveBuffer(&((*handle)->receiveBuffer));
    free(*handle);
    (*handle) = NULL;
}