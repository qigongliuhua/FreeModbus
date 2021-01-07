


#include "mdregpool.h"
#include <stdlib.h>


static mdU32 start_addr[] = {  REGISTER_OFFSET +  COIL_OFFSET,
                        REGISTER_OFFSET +  INPUT_COIL_OFFSET,
                        REGISTER_OFFSET +  INPUT_REGISTER_OFFSET,
                        REGISTER_OFFSET +  KEEP_REGISTER_OFFSET,};





static mdSTATUS mdCreateRegister(RegisterPoolHandle handle,RegisterHandle* reg){
    mdSTATUS ret = mdFALSE;
    (*reg) = NULL;
    if (handle->curRegisterNumber < handle->maxRegisterNumber)
    {
        RegisterHandle reghandle = (RegisterHandle)malloc(sizeof(struct Register));
        if (reghandle != NULL)
        {
            reghandle->addr = 0;
            reghandle->data = 0;
            reghandle->next = NULL;
            (*reg) = reghandle;
            handle->curRegisterNumber++;
            ret = mdTRUE;
        }
    }
    return ret;
}

static mdVOID mdDestoryRegister(RegisterPoolHandle handle,RegisterHandle* reg){
    free(*reg);
    handle->curRegisterNumber--;
    (*reg) = NULL;
}


static mdSTATUS mdFindRegisterByAddress(RegisterPoolHandle handle, mdU32 addr, RegisterHandle *reg){
    mdSTATUS ret = mdFALSE;
    for (mdU32 i = 0; i < 4; i++)
    {
        if((addr >= start_addr[i]) && (addr < start_addr[i] + REGISTER_POOL_MAX_BUFFER)){
            (*reg) = handle->quickMap[i][addr-start_addr[i]];
            ret = mdTRUE;
        }
    }
    if (ret == mdFALSE)
    {
        RegisterHandle p = handle->pool->next;
        while (p != NULL)
        {
            if (addr == p->addr)
            {
                (*reg) = p;
                ret = mdTRUE;
                break;
            }
            else if (addr < p->addr)
            {
                (*reg) = NULL;
                break;
            }
            else p=p->next;  
        } 
    }
    return ret;
}


static mdSTATUS mdInsertRegister(RegisterPoolHandle handle,RegisterHandle* reg){
    mdSTATUS ret = mdFALSE;
    RegisterHandle p=handle->pool,q=p->next;
    while (q!=NULL && q->addr < (*reg)->addr){
        p = q;
        q = p->next;
    }
    if (q == NULL)
    {
        p->next = *reg;
        (*reg)->next = NULL;
        ret = mdTRUE;
    }
    else if(q->addr == (*reg)->addr){
        ret = mdFALSE;
    }
    else{
        (*reg)->next = q;
        p->next = *reg;
        ret = mdTRUE;
    }
    return ret;
}

#define mdToDouble(n) ((double)n)
#define mdREG_ADDR(n) ((mdU32)(mdToDouble(n)/REGISTER_WIDTH))
#define mdREG_OFFSET(n) (n%REGISTER_WIDTH)
#define mdGetBit(reg,offset) ((reg>>offset)&1)
#define ToBit(n) ((mdU32)n>0?mdHigh:mdLow)

static mdSTATUS mdReadBit(RegisterPoolHandle handle,mdU32 addr,mdBit *bit){
    mdSTATUS ret = mdFALSE;
    mdU32 reg_addr = mdREG_ADDR(addr) + REGISTER_OFFSET;    mdU32 reg_off = mdREG_OFFSET(addr);
    RegisterHandle reg;
    ret = mdFindRegisterByAddress(handle,reg_addr,&reg);
    if (ret == mdTRUE)
    {
        (*bit) = ToBit( mdGetBit(reg->data,reg_off));
    }
    else{
        (*bit) = mdLow;
    }

    return mdTRUE;
}
static mdSTATUS mdWriteBit(RegisterPoolHandle handle,mdU32 addr,mdBit bit){
    mdSTATUS ret = mdFALSE;
    mdU32 reg_addr = mdREG_ADDR(addr) + REGISTER_OFFSET;
    mdU32 reg_off = mdREG_OFFSET(addr);
    RegisterHandle reg;
    ret = mdFindRegisterByAddress(handle,reg_addr,&reg);
    bit = ToBit(bit);
    if (ret == mdTRUE)    {
        reg->data |= (bit << reg_off);
    }
    else{
        ret = mdCreateRegister(handle,&reg);
        if (ret == mdTRUE)
        {
            reg->addr = reg_addr;
            reg->data |= (bit << reg_off);
            ret = mdInsertRegister(handle,&reg);
        }   
    }
    return ret;
}
static mdSTATUS mdReadBits(RegisterPoolHandle handle,mdU32 addr,mdU32 len,mdBit *bits){
    mdSTATUS ret = mdFALSE;
    for (mdU32 i = 0; i < len; i++)
    {
        ret = handle->mdReadBit(handle,addr+i, bits++);
    }
    return ret;

}

static mdSTATUS mdWriteBits(RegisterPoolHandle handle,mdU32 addr,mdU32 len,mdBit* bits){
    mdSTATUS ret = mdFALSE;
    for (mdU32 i = 0; i < len; i++)
    {
        ret = handle->mdWriteBit(handle,addr+i,bits[i]);
    }
    return ret;
}

static mdSTATUS mdReadU16(RegisterPoolHandle handle,mdU32 addr,mdU16 *data){
    mdSTATUS ret = mdFALSE;
    RegisterHandle reg;
    ret = mdFindRegisterByAddress(handle,addr,&reg);
    if (ret == mdTRUE)
    {
        (*data) = reg->data; 
    }
    else{
        (*data) = 0;
    }
    return mdTRUE;
}

static mdSTATUS mdWriteU16(RegisterPoolHandle handle,mdU32 addr,mdU16 data){
    mdSTATUS ret = mdFALSE;
    RegisterHandle reg;
    ret = mdFindRegisterByAddress(handle,addr,&reg);
    if (ret == mdTRUE)
    {
        reg->data = data;
    }
    else{
        ret = mdCreateRegister(handle,&reg);
        if (ret == mdTRUE)
        {
            reg->addr = addr;
            reg->data = data;
            ret = mdInsertRegister(handle,&reg);
        }   
    }
    return ret;
}

static mdSTATUS mdReadU16s(RegisterPoolHandle handle,mdU32 addr,mdU32 len,mdU16 *data){
    mdSTATUS ret = mdFALSE;
    for (mdU32 i = 0; i < len; i++)
    {
        ret = handle->mdReadU16(handle,addr+i, data++);
    }
    return ret;
}

static mdSTATUS mdWriteU16s(RegisterPoolHandle handle,mdU32 addr,mdU32 len,mdU16 *data){
    mdSTATUS ret = mdFALSE;
    for (mdU32 i = 0; i < len; i++)
    {
        ret = handle->mdWriteU16(handle,addr+i,*(data++));
    }
    return ret;
}



mdSTATUS mdCreateRegisterPool(RegisterPoolHandle* regpoolhandle){
    mdSTATUS ret = mdFALSE;
    RegisterPoolHandle handle;
    handle= (RegisterPoolHandle)malloc(sizeof(struct RegisterPool));
    if (handle != NULL)
    {
        //注册方法
        handle->mdReadBit = mdReadBit;
        handle->mdReadBits = mdReadBits;
        handle->mdReadU16 = mdReadU16;
        handle->mdReadU16s = mdReadU16s;
        handle->mdWriteBit = mdWriteBit;
        handle->mdWriteBits = mdWriteBits;
        handle->mdWriteU16 = mdWriteU16;
        handle->mdWriteU16s = mdWriteU16s;

        //设定最大寄存器数量
        handle->maxRegisterNumber = REGISTER_POOL_MAX_REGISTER_NUMBER;

        
        //采用头节点模式
        ret = mdCreateRegister(handle,&(handle->pool));     
        if (ret == mdFALSE) goto exit;

        //构建寄存器池和快表
        RegisterHandle p = handle->pool;
        for (mdU32 i = 0; i < 4; i++)
        {
            for (mdU32 j = 0; j < REGISTER_POOL_MAX_BUFFER; j++)
            {
                RegisterHandle reg;
                ret = mdCreateRegister(handle,&reg);
                if (ret == mdFALSE) goto exit;
                reg->addr = start_addr[i] + j;
                handle->quickMap[i][j] = reg;       
                p->next = reg;
                p = reg;              
            }  
        }
        ret = mdTRUE;
    }
exit:    (*regpoolhandle) = handle;
    return ret;
}
mdVOID mdDestoryRegisterPool(RegisterPoolHandle* regpoolhandle){

    //释放所有寄存器
    RegisterHandle p = (*regpoolhandle)->pool;
    while (p->next != NULL)
    {
        RegisterHandle q = p->next;
        mdDestoryRegister(*regpoolhandle,&p);
        p = q;
    }
    //释放寄存器池
    free(*regpoolhandle);
    (*regpoolhandle) = NULL;
}