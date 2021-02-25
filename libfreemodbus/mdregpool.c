


#include "mdregpool.h"
#include <stdlib.h>



/* ================================================================== */
/*                        底层代码                                     */
/*              作用：实现动态增删任意地址寄存器，加速寄存器访问               */
/* ================================================================== */

static mdU32 start_addr[] = {   REGISTER_OFFSET +  COIL_OFFSET,
                                REGISTER_OFFSET +  INPUT_COIL_OFFSET,
                                REGISTER_OFFSET +  INPUT_REGISTER_OFFSET,
                                REGISTER_OFFSET +  KEEP_REGISTER_OFFSET,};




/*
    mdCreateRegister
        @handle 句柄
        @reg    创建的寄存器
        @return 成功返回 mdTRUE，失败返回 mdFALSE
    分配一个寄存器，初始化它，句柄的寄存器数量加一。如果当前寄存器数量超过设定的最大寄存器数量，则分配失败
*/
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

/*
    mdDestoryRegister
        @handle 句柄
        @reg    需要删除的寄存器
        @return
    释放寄存器占用的空间，并且将句柄中的 curRegisterNumber 减一
*/
static mdVOID mdDestoryRegister(RegisterPoolHandle handle,RegisterHandle* reg){
    free(*reg);
    handle->curRegisterNumber--;
    (*reg) = NULL;
}

/*
    mdFindRegisterByAddress
        @handle 句柄
        @addr   寄存器地址
        @reg    返回找到的寄存器
        @return 找到则返回 mdTRUE，且填充 reg，否则返回 mdFALSE，reg置为 NULL
    根据寄存器地址寻找寄存器，若找到则返回 mdTRUE，且填充 reg，否则返回 mdFALSE，reg置为 NULL
*/
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


/*
    mdInsertRegister
        @handle 句柄
        @reg    插入的寄存器
        @return 插入成功返回 mdTRUE， 否则返回 mdFALSE
    遍历寄存器池，按照寄存器地址大小顺序插入适当的位置，如果已存在则返回 mdFALSE
*/
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


/* ================================================================== */
/*                        第二层封装                                    */
/*     作用：实现理论上(其实限制寄存器最大数量)寄存器数量无限多，               */
/*            忽略不用寄存器地址存储的困扰                                 */
/* ================================================================== */

#define mdToDouble(n) ((double)n)
#define mdREG_ADDR(n) ((mdU32)(mdToDouble(n)/REGISTER_WIDTH))
#define mdREG_OFFSET(n) (n%REGISTER_WIDTH)
#define mdGetBit(reg,offset) ((reg>>offset)&1)
#define ToBit(n) ((mdU32)n>0?mdHigh:mdLow)

/*
    mdReadBit
        @handle 句柄
        @addr    位地址，如果寄存器位宽为16，则0~15都在第一个寄存器中，以此类推
        @bit    位结果
        @return mdTRUE
    根据地址在当前句柄中读取位大小
*/
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

/*
    mdWriteBit
        @handle 句柄
        @addr    位地址，如果寄存器位宽为16，则0~15都在第一个寄存器中，以此类推
        @bit    位大小
        @return 空间不足时返回 mdFalse，否则 mdTRUE
    根据地址修改当前句柄中的位大小
*/
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

/*
    mdReadBits
        @handle 句柄
        @addr    位地址，如果寄存器位宽为16，则0~15都在第一个寄存器中，以此类推
        @len    长度
        @bits    位数组
        @return mdTRUE
    根据地址在当前句柄中读取 len 个位大小，结果保存在 bits中
*/
static mdSTATUS mdReadBits(RegisterPoolHandle handle,mdU32 addr,mdU32 len,mdBit *bits){
    mdSTATUS ret = mdFALSE;
    for (mdU32 i = 0; i < len; i++)
    {
        ret = handle->mdReadBit(handle,addr+i, bits++);
    }
    return ret;

}

/*
    mdReadBits
        @handle 句柄
        @addr    位地址，如果寄存器位宽为16，则0~15都在第一个寄存器中，以此类推
        @len    长度
        @bits    位数组
        @return 空间不足时返回 mdFalse，否则 mdTRUE
    根据地址修改当前句柄中的 len 个位大小
*/
static mdSTATUS mdWriteBits(RegisterPoolHandle handle,mdU32 addr,mdU32 len,mdBit* bits){
    mdSTATUS ret = mdFALSE;
    for (mdU32 i = 0; i < len; i++)
    {
        ret = handle->mdWriteBit(handle,addr+i,bits[i]);
    }
    return ret;
}

/*
    mdReadU16
        @handle 句柄
        @addr    寄存器地址
        @data    寄存器数据
        @return  mdTRUE
    根据地址读取一个寄存器值
*/
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


/*
    mdReadU16
        @handle 句柄
        @addr    寄存器地址
        @data    寄存器数据
        @return  空间不足时返回 mdFALSE ,否则返回 mdTRUE
    根据地址写入一个寄存器
*/
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

/*
    mdReadU16
        @handle 句柄
        @addr    寄存器地址
        @len    读取长度
        @data    值数组
        @return  mdTRUE
    根据地址读取一组寄存器值
*/
static mdSTATUS mdReadU16s(RegisterPoolHandle handle,mdU32 addr,mdU32 len,mdU16 *data){
    mdSTATUS ret = mdFALSE;
    for (mdU32 i = 0; i < len; i++)
    {
        ret = handle->mdReadU16(handle,addr+i, data++);
    }
    return ret;
}


/*
    mdWriteU16s
        @handle 句柄
        @addr    寄存器地址
        @len    写入长度
        @data    值数组
        @return  空间不足时返回 mdFALSE ,否则返回 mdTRUE
    根据地址写入一组寄存器值
*/
static mdSTATUS mdWriteU16s(RegisterPoolHandle handle,mdU32 addr,mdU32 len,mdU16 *data){
    mdSTATUS ret = mdFALSE;
    for (mdU32 i = 0; i < len; i++)
    {
        ret = handle->mdWriteU16(handle,addr+i,*(data++));
    }
    return ret;
}

/*
    mdCreateRegisterPool
        @regpoolhandle  句柄
        @return 空间不足时返回 mdFALSE，否则返回 mdTRUE
    创建并初始化寄存器池
*/
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

/*
    mdDestoryRegisterPool
        @regpoolhandle  句柄
        @return
    释放寄存器池空间，句柄置 NULL
*/
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