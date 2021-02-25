
#include "mdrecbuffer.h"
#include <stdlib.h>
#include <string.h>

/*
    mdClearReceiveBuffer
        @handle 句柄
        @return
    复位接收缓冲
*/
mdVOID mdClearReceiveBuffer(ReceiveBufferHandle handle)
{
    handle->count = 0;
    memset(handle->buf,0,MODBUS_PDU_SIZE_MAX);
}

/*
    mdCreateReceiveBuffer
        @handle 句柄
        @return
    创建并初始化接收缓冲
*/
mdSTATUS mdCreateReceiveBuffer(ReceiveBufferHandle *handle)
{
    (*handle) = (ReceiveBufferHandle)malloc(sizeof(struct ReceiveBuffer));
    if(!handle){
        free(handle);
        return mdFALSE;
    }
    mdClearReceiveBuffer(*handle);
    return mdTRUE;
}

/*
    mdDestoryReceiveBuffer
        @handle 句柄
        @return
    销毁接收缓冲，释放内存
*/
mdVOID mdDestoryReceiveBuffer(ReceiveBufferHandle *handle)
{
    free(*handle);
    (*handle) = NULL;
}

