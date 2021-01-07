
#include <stdlib.h>
#include <string.h>
#include "modbus.h"

#define LOW(n) ((mdU16)n%256)
#define HIGH(n) ((mdU16)n/256)
#define ToDouble(n) ((double)n)
#define ToU16(high,low) ((((mdU16)high & 0x00ff)<<8) | \
                            ((mdU16)low & 0x00ff))
/*
#define FrameToBuffer() buf[count++] = frame->id;\
                        buf[count++] = frame->fcode;\
                        memcpy(&buf[2],frame->payload,frame->plcount);\
                        count += frame->plcount;\
                        buf[count++] = LOW(frame->crc16);\
                        buf[count++] = HIGH(frame->crc16)
*/

#define OutTime ((mdU32)(((8*3.5) / ToDouble(hander->usartbaud))*10000))
#define LossTime ((mdU32)(((8*1.5) / ToDouble(hander->usartbaud))*10000))
/*
#define ResetReceBuf()  memset((void*)(hander->receBuf),0,MODBUS_PDU_SIZE_MAX);\
                        hander->receCount = 0;         \
                        hander->receTime = 0;\
                        hander->receStatus = Ready

*/
static mdVOID mdClearReceiveBuffer(ReceiveBufferHandle handle);
static mdVOID mdRtuBaseTimerTick(ModbusSlaveHandle handle, mdU32 time);

static mdVOID portRtuDriver(ModbusSlaveHandle handle){


}
static mdVOID portRtuPushChar(ModbusSlaveHandle handle,mdU8 c){
    ReceiveBufferHandle recbuf = handle->receiveBuffer;
    recbuf->buf[recbuf->count++] = c;
    if (c)
    {
        /* code */
    }
}

static mdVOID portRtu1000usTimerTick(ModbusSlaveHandle handle){
    mdRtuBaseTimerTick(handle,1000);
}
static mdVOID portRtu100usTimerTick(ModbusSlaveHandle handle){
    mdRtuBaseTimerTick(handle,100);
}
static mdVOID portRtu10usTimerTick(ModbusSlaveHandle handle){
    mdRtuBaseTimerTick(handle,10);
}
static mdVOID portRtu1usTimerTick(ModbusSlaveHandle handle){
    mdRtuBaseTimerTick(handle,1);
}

static mdVOID mdRtuBaseTimerTick(ModbusSlaveHandle handle, mdU32 time){
    

}

static mdVOID mdClearReceiveBuffer(ReceiveBufferHandle handle){
    handle->status = ReceiveStatus_Ready;
    handle->count = 0;
    handle->time = 0;
    memset(handle->buf,0,MODBUS_PDU_SIZE_MAX);
}

static mdVOID mdCreateReceiveBuffer(ReceiveBufferHandle *handle){
    (*handle) = (ReceiveBufferHandle)malloc(sizeof(struct ReceiveBuffer));
    mdClearReceiveBuffer(*handle);
}



static mdVOID mdModbusCenterProcessor(ModbusSlaveHandle handle,ModbusRtuFrameHandle receFrame){


}


mdSTATUS mdCreateModbusSlave(ModbusSlaveHandle *handle, RtuPopCharHandle mdRtuPopChar){
    mdSTATUS ret = mdFALSE;
    ModbusSlaveHandle slaveHandle = (ModbusSlaveHandle)malloc(sizeof(struct ModbusSlave));
    if (slaveHandle != NULL)
    {
        slaveHandle->portRtuDriver = portRtuDriver;
        slaveHandle->portRtuPushChar = portRtuPushChar;
        slaveHandle->portRtu1000usTimerTick = portRtu1000usTimerTick;
        slaveHandle->portRtu100usTimerTick = portRtu100usTimerTick;
        slaveHandle->portRtu10usTimerTick = portRtu10usTimerTick;
        slaveHandle->portRtu1usTimerTick = portRtu1usTimerTick;
        slaveHandle->mdRtuPopChar = mdRtuPopChar;

        ret = mdCreateRegisterPool(&(slaveHandle->registerPool));
        mdCreateReceiveBuffer(&(slaveHandle->receiveBuffer));
    }
    return ret;
}



mdVOID mdDestoryModbusSlave(ModbusSlaveHandle *handle){
    mdDestoryRegisterPool(&((*handle)->registerPool));
    free((*handle)->receiveBuffer);
    free(*handle);
    (*handle) = NULL;
}


/*

static mdVOID PushFrame(ModbusHander hander, MDRTUFrameHander frame, ListFunctionMode mode){
    if (mode == RECEIVE_BUFFER)
    {
        hander->rfTail->next = frame;
        hander->rfTail = frame;
    }
    else
    {
        hander->sfTail->next = frame;
        hander->sfTail = frame;
    } 
}

static mdSTATUS PopFrame(ModbusHander hander,MDRTUFrameHander* frame,ListFunctionMode mode){
    mdBOOL ret = mdFALSE;
    if (mode == RECEIVE_BUFFER)
    {
        if (hander->rfHeader != hander->rfTail)
        {
            (*frame) = hander->rfHeader;
            hander->rfHeader = (*frame)->next;
            ret = mdTRUE; 
        }       
    }
    else
    {
        if (hander->sfHeader != hander->sfTail)
        {
            (*frame) = hander->sfHeader;
            hander->sfHeader = (*frame)->next;
            ret = mdTRUE; 
        } 
    } 
}

static mdVOID FrameCreate(ModbusHander hander,MDRTUFrameHander *frame){
    (*frame) = (MDRTUFrameHander)malloc(sizeof(MDRTUFrame));
}

static mdVOID FrameDestory(ModbusHander hander,MDRTUFrameHander *frame){
    free((*frame)->payload);
    free((*frame));
    (*frame) = NULL;
}





static mdVOID HandleReceiveFrame(ModbusHander hander){
    MDRTUFrameHander frame;
    mdBOOL ret;
    ret = hander->PopFrame(hander,&frame,RECEIVE_BUFFER);
    if (ret == mdFALSE)
    {
        return;
    }
#ifdef IGNORE_LOSS_FRAME
    if((frame->status & OutMaxLen)!=0 && (frame->status & Loss)!=0){
#else
    if((frame->status & OutMaxLen)!=0){
#endif
        hander->runcount.error++;
        hander->FrameDestory(hander,&frame);
        return;
    }
#ifdef IGNORE_CRC_CHECK
    if ((frame->status & CRCError) !=0 )
    {
        hander->runcount.error++;
        hander->FrameDestory(hander,&frame);
        return;
    }
#endif
    if (frame->id != hander->slaveId)
    {
        hander->runcount.error++;
        hander->FrameDestory(hander,&frame);
        return;
    }

    MDRTUFrameHander sendframe;
    hander->FrameCreate(hander,&sendframe);
    switch (frame->fcode)
    {
        case MODBUS_CODE_1:
        case MODBUS_CODE_2:
            

            break;
        case MODBUS_CODE_3:
                
            break;
        case MODBUS_CODE_4:
                
            break;
        case MODBUS_CODE_5:
                
            break;
        case MODBUS_CODE_6:
                
            break;
        case MODBUS_CODE_15:
                
            break;
        case MODBUS_CODE_16:
                
            break;
        default:
            break;
    }

    hander->FrameDestory(hander,&frame);
}

static mdVOID ModbusRTUSendString(ModbusHander hander, mdU8* buf,mdU32 len){
    for (mdU32 i = 0; i < len; i++)
    {
        hander->mdSendChar(hander,buf[i]);
    }
}


static mdVOID HandleSendFrame(ModbusHander hander){
    MDRTUFrameHander frame;
    mdBOOL ret;
    mdU8* buf;
    mdU32 count;
    ret = hander->PopFrame(hander,&frame,SEND_BUFFER);
    if (ret == mdTRUE)
    {
        buf = (mdU8*)malloc(MODBUS_PDU_SIZE_MAX + 2);
        FrameToBuffer();
        hander->ModbusRTUSendString(hander,buf,count);
    }
}




static mdSTATUS CreateReceFrame(ModbusHander hander){
    mdSTATUS ret =mdFALSE;
    MDRTUFrameHander frame;
    mdU32 len = hander->receCount;
    if (len >= MODBUS_PDU_SIZE_MIN)
    {
        mdU16 crc16 = ToU16(hander->receBuf[len-1],
                            hander->receBuf[len-2]);

        if (crc16 != mdCrc16(hander->receBuf,hander->receCount-2))
        {
            hander->receStatus |= CRCError;
        }

        hander->FrameCreate(hander,&frame);
        frame->id = hander->receBuf[0];
        frame->fcode = hander->receBuf[1];
        frame->crc16 = crc16;
        frame->plcount = len - 4;
        frame->payload = (mdU8*)malloc(sizeof(mdU8)*frame->plcount);
        memcpy(frame->payload,&(hander->receBuf[2]),frame->plcount);
        frame->status = hander->receStatus; 
        hander->PushFrame(hander,frame,RECEIVE_BUFFER);
        ResetReceBuf(); 
        ret = mdTRUE; 
    }
    return ret;
}

ModbusHander ModbusRTUCreateSlave(mdSendCharPointer usartSendChar){
    ModbusHander hander =  (ModbusHander)malloc(sizeof(Modbus));
    if (hander)
    {
        hander->mdSendChar = usartSendChar;
    }
}






mdVOID portRtuTimer100usTick(ModbusHander hander){
    if (hander->receCount > 1)
    {
        if(++(hander->receTime) > LossTime){
            hander->receStatus |= Loss;
        }
        else if (hander->receTime > OutTime)
        {
            hander->receStatus |= Normal;
            hander->CreateReceFrame(hander);
        }   
    }
}

 

mdVOID portReceiveChar(ModbusHander hander,mdU8 c){

    if (hander->receCount < MODBUS_PDU_SIZE_MAX)
    {
        hander->receBuf[hander->receCount++] = c;
        if (hander->receCount >= MODBUS_PDU_SIZE_MAX)
        {
            hander->receStatus |= OutMaxLen;
            hander->CreateReceFrame(hander);
        }
    }
    
}


mdVOID portModbusRtuDriver(ModbusHander hander){
    hander->HandleReceiveFrame(hander);
    hander->HandleSendFrame(hander);
}




*/