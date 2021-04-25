# FreeModbus

## 介绍
简单易用的Modbus RTU库，可以同时运行几个独立的Modbus RTU从机。

## 使用说明

### 1.搭建从机

#### 1.1.创建Modbus RTU从机

```c
   #include "mdrtuslave.h"

   #define SLAVE_ID 5
   #define BUAD_RATE 9600
   static ModbusRTUSlaveHandler mdhandler;

   static mdVOID popchar(ModbusRTUSlaveHandler handler, mdU8 c)
   {
      // 此处添加串口发送代码
   }

   static void ModbusInit()
   {

      struct ModbusRTUSlaveRegisterInfo info;
      info.slaveId = SLAVE_ID;
      info.usartBaudRate = BUAD_RATE;
      info.mdRTUPopChar = popchar;
      mdCreateModbusRTUSlave(&mdhandler,info);
   }

   ModbusInit();
```

#### 1.2.调用心跳函数

```c
   #define TIMER_UTIME  100  //定时器周期为100us

   static void timer_handler(void)  //100us定时器回调函数
   {
      mdhandler->portRTUTimerTick(mdhandler, TIMER_UTIME);
   }
   /*函数作用：出*/
```

#### 1.3.从串口接收数据

```c
   static void usart_rec_handler(char *buf, size_t len)   //串口接收中断函数
   {
      for (size_t i = 0; i < len; i++)
      {
         mdhandler->portRTUPushChar(mdhandler,buf[i]);
      }
   }
   /*函数作用：入*/
```

### 2.写入和读取从机寄存器

#### 2.1.读取输入线圈

```c
   mdBit bit;
   mdU32 addr = 0;
   mdSTATUS ret;
   ret = mdhandler->registerPool->mdReadInputCoil(mdhandler->registerPool, addr, &bit);   //读取输入线圈，设备地址为10001
   if(ret == mdFALSE)
   {
      printf("读取失败\n");
   }
```

#### 2.2.读取保持寄存器

```c
   mdU16 data;
   mdU32 addr = 0;
   mdSTATUS ret;
   ret = mdhandler->registerPool->mdReadHoldRegister(mdhandler->registerPool, addr, &data);    //读取保持寄存器，地址为40001
   if(ret == mdFALSE)
   {
      printf("读取失败\n");
   }
```

#### 2.3.写入输入线圈

```c
   mdBit bit = mdHigh;
   mdU32 addr = 0;
   mdSTATUS ret;
   ret = mdhandler->registerPool->mdWriteInputCoil(mdhandler->registerPool, addr, bit);   //写入输入线圈，地址为10001，高电平
   if(ret == mdFALSE)
   {
      printf("写入失败\n");
   }
```

#### 2.4.写入保持寄存器

```c
   mdU16 data = 0x1234;
   mdU32 addr = 0;
   mdSTATUS ret;
   ret = mdhandler->registerPool->mdWriteHoldRegister(mdhandler->registerPool, addr, data);  //写入保持寄存器，地址为40001，大小为0x1234
   if(ret == mdFALSE)
   {
      printf("写入失败\n");
   }
```

#### 2.5.其他寄存器操作API

```c
   /*不推荐*/
    mdSTATUS (*mdReadBit)(RegisterPoolHandle handler,mdU32 addr,mdBit *bit);
    mdSTATUS (*mdWriteBit)(RegisterPoolHandle handler,mdU32 addr,mdBit bit);
    mdSTATUS (*mdReadBits)(RegisterPoolHandle handler,mdU32 addr,mdU32 len,mdBit *bits);
    mdSTATUS (*mdWriteBits)(RegisterPoolHandle handler,mdU32 addr,mdU32 len,mdBit *bits);
    mdSTATUS (*mdReadU16)(RegisterPoolHandle handler,mdU32 addr,mdU16 *data);
    mdSTATUS (*mdWriteU16)(RegisterPoolHandle handler,mdU32 addr,mdU16 data);
    mdSTATUS (*mdReadU16s)(RegisterPoolHandle handler,mdU32 addr,mdU32 len,mdU16 *data);
    mdSTATUS (*mdWriteU16s)(RegisterPoolHandle handler,mdU32 addr,mdU32 len,mdU16 *data);

   /*推荐*/
    mdSTATUS (*mdReadCoil)(RegisterPoolHandle handler, mdU32 addr, mdBit* bit);  //设备地址1～10000，addr=0 -> 设备1
    mdSTATUS (*mdReadCoils)(RegisterPoolHandle handler, mdU32 addr, mdU32 len, mdBit* bits);
    mdSTATUS (*mdWriteCoil)(RegisterPoolHandle handler, mdU32 addr, mdBit bit);
    mdSTATUS (*mdWriteCoils)(RegisterPoolHandle handler, mdU32 addr, mdU32 len, mdBit* bits);
    mdSTATUS (*mdReadInputCoil)(RegisterPoolHandle handler, mdU32 addr, mdBit* bit);   //设备地址10001～20000，addr=0 -> 设备10001
    mdSTATUS (*mdReadInputCoils)(RegisterPoolHandle handler, mdU32 addr, mdU32 len, mdBit* bits);
    mdSTATUS (*mdWriteInputCoil)(RegisterPoolHandle handler, mdU32 addr, mdBit bit);
    mdSTATUS (*mdWriteInputCoils)(RegisterPoolHandle handler, mdU32 addr, mdU32 len, mdBit* bits);
    mdSTATUS (*mdReadInputRegister)(RegisterPoolHandle handler, mdU32 addr, mdU16* data);//设备地址30001～40000，addr=0 -> 设备30001
    mdSTATUS (*mdReadInputRegisters)(RegisterPoolHandle handler, mdU32 addr, mdU32 len, mdU16* data);
    mdSTATUS (*mdWriteInputRegister)(RegisterPoolHandle handler, mdU32 addr, mdU16 data);
    mdSTATUS (*mdWriteInputRegisters)(RegisterPoolHandle handler, mdU32 addr, mdU32 len, mdU16* data);
    mdSTATUS (*mdReadHoldRegister)(RegisterPoolHandle handler, mdU32 addr, mdU16* data);//设备地址40001～50000，addr=0 -> 设备40001
    mdSTATUS (*mdReadHoldRegisters)(RegisterPoolHandle handler, mdU32 addr, mdU32 len, mdU16* data);
    mdSTATUS (*mdWriteHoldRegister)(RegisterPoolHandle handler, mdU32 addr, mdU16 data);
    mdSTATUS (*mdWriteHoldRegisters)(RegisterPoolHandle handler, mdU32 addr, mdU32 len, mdU16* data);
```

## Modbus变量地址

|映射地址|Function Code|地址类型|R/W|描述|
|---|---|---|---|---|
|0xxxx|01,05,15|线圈|R/W|-|
|1xxxx|02|输入线圈|R|-|
|3xxxx|04|输入寄存器|R|每个寄存器表示一个16-bit无符号整数|（0~65535）|
|4xxxx|03,06,16|保持寄存器|R/W|-|

## 更新计划

- [x] Code1
- [x] Code2
- [x] Code3
- [x] Code4
- [x] Code5
- [x] Code6
- [x] Code15
- [x] Code16
- [ ] 实机测试

## 注意

有亿点懒，不定期更新
