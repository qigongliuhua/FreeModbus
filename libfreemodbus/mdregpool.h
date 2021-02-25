#ifndef __MDREGPOOL_H__
#define __MDREGPOOL_H__


#include "mdtype.h"
#include "mdconfig.h"


typedef struct Register* RegisterHandle;
struct Register
{
    mdU32 addr;
    mdU32 data;
    RegisterHandle next;
};

typedef struct RegisterPool* RegisterPoolHandle;
struct RegisterPool
{
    RegisterHandle pool;
    //线圈、输入状态、输入寄存器、保持寄存器
    RegisterHandle quickMap[4][REGISTER_POOL_MAX_BUFFER];
    mdU32 curRegisterNumber,maxRegisterNumber;

    mdSTATUS (*mdReadBit)(RegisterPoolHandle handle,mdU32 addr,mdBit *bit);
    mdSTATUS (*mdWriteBit)(RegisterPoolHandle handle,mdU32 addr,mdBit bit);
    mdSTATUS (*mdReadBits)(RegisterPoolHandle handle,mdU32 addr,mdU32 len,mdBit *bits);
    mdSTATUS (*mdWriteBits)(RegisterPoolHandle handle,mdU32 addr,mdU32 len,mdBit *bits);
    mdSTATUS (*mdReadU16)(RegisterPoolHandle handle,mdU32 addr,mdU16 *data);
    mdSTATUS (*mdWriteU16)(RegisterPoolHandle handle,mdU32 addr,mdU16 data);
    mdSTATUS (*mdReadU16s)(RegisterPoolHandle handle,mdU32 addr,mdU32 len,mdU16 *data);
    mdSTATUS (*mdWriteU16s)(RegisterPoolHandle handle,mdU32 addr,mdU32 len,mdU16 *data);
};


mdExport mdSTATUS mdCreateRegisterPool(RegisterPoolHandle* regpoolhandle);
mdExport mdVOID mdDestoryRegisterPool(RegisterPoolHandle* regpoolhandle);

#define mdGetBit(reg,offset) ((reg>>offset)&1)
#define mdSetBit(handle,offset,bit) do{(handle->data) |= (bit << offset);}while(0)

#endif