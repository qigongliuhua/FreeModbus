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
   mdU32 addr = 10001;
   mdSTATUS ret;
   ret = mdhandler->registerPool->mdReadBit(mdhandler->registerPool, addr, &bit);   //读取输入线圈，地址为10001
   if(ret == mdFALSE)
   {
      printf("读取失败\n");
   }
```

#### 2.2.读取保持寄存器

```c
   mdU16 data;
   mdU32 addr = 40001;
   mdSTATUS ret;
   ret = mdhandler->registerPool->mdReadU16(mdhandler->registerPool, addr, &data);    //读取保持寄存器，地址为40001
   if(ret == mdFALSE)
   {
      printf("读取失败\n");
   }
```

#### 2.3.写入输入线圈

```c
   mdBit bit = mdHigh;
   mdU32 addr = 10001;
   mdSTATUS ret;
   ret = mdhandler->registerPool->mdWriteBit(mdhandler->registerPool, addr, bit);   //写入输入线圈，地址为10001，高电平
   if(ret == mdFALSE)
   {
      printf("写入失败\n");
   }
```

#### 2.4.写入保持寄存器

```c
   mdU16 data = 0x1234;
   mdU32 addr = 40001;
   mdSTATUS ret;
   ret = mdhandler->registerPool->mdWriteU16(mdhandler->registerPool, addr, data);  //写入保持寄存器，地址为40001，大小为0x1234
   if(ret == mdFALSE)
   {
      printf("写入失败\n");
   }
```

#### 2.5.其他寄存器操作API

```c
   mdReadBits()
   mdReadU16s()
   mdWriteBits()
   mdWriteU16s()
```

## 更新计划

- [x] Code1
- [ ] Code2
- [ ] Code3
- [ ] Code4
- [ ] Code5
- [ ] Code6
- [ ] Code15
- [ ] Code16

## 注意

有亿点懒，不定期更新
