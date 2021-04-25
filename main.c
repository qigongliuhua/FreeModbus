
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "mdrtuslave.h"
#include "core.h"

#define BUAD_RATE 9600
#define SLAVE_ID 5

#define BYTE_UTIME ((mdU32)(8000000 / BUAD_RATE))

ptimer timer;
static ModbusRTUSlaveHandler mdhandler;
static pthread_mutex_t mut;
static pthread_cond_t cond;

static pthread_mutex_t mdmux;

static void timer_handler(void)
{
    int ret = 0;
    pthread_mutex_lock(&mdmux);
    mdhandler->portRTUTimerTick(mdhandler, BYTE_UTIME);
    pthread_mutex_unlock(&mdmux);
}

static mdVOID popchar(ModbusRTUSlaveHandler handler, mdU8 c)
{
    char buf[12];
    sprintf(buf, "%02X ", c);
    for (int i = 0; i < strlen(buf); i++)
    {
        putc(buf[i],stdout);
    }
    fflush(stdout);
}

static void usart_send(char *buf, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        pthread_mutex_lock(&mdmux);
        mdhandler->portRTUPushChar(mdhandler,buf[i]);
        pthread_mutex_unlock(&mdmux);
        usleep(BYTE_UTIME);
    }
}

#define TEST 16

static void sendtest()
{
#if TEST==1 //pass
    char str1[] = {SLAVE_ID, 0x01, 0x00, 0x00, 0x00, 0x09, 0x88, 0xfd};
    usart_send(str1, sizeof(str1)/sizeof(char));
    sleep(2);
#endif

#if TEST==2 //pass
    char str2[] = {SLAVE_ID, 0x02, 0x00, 0x00, 0x00, 0x09, 0x88, 0xb9};
    usart_send(str2, sizeof(str2)/sizeof(char));
    sleep(2);
#endif

#if TEST==3 //pass
    char str3[] = {SLAVE_ID, 0x03, 0x00, 0x00, 0x00, 0x02, 0x8f, 0xc5};
    usart_send(str3, sizeof(str3)/sizeof(char));
    sleep(2);
#endif

#if TEST==4 //pass
    char str4[] = {SLAVE_ID, 0x04, 0x00, 0x00, 0x00, 0x02, 0x4f, 0x70};
    usart_send(str4, sizeof(str4)/sizeof(char));
    sleep(2);
#endif

#if TEST==5 //pass
    char str5[] = {SLAVE_ID, 0x05, 0x00, 0x00, 0x00, 0x00, 0x4e, 0xcc};
    usart_send(str5, sizeof(str5)/sizeof(char));
    sleep(2);
    mdU16 data;
    mdhandler->registerPool->mdReadU16(mdhandler->registerPool, 0 + COIL_OFFSET, &data);
    if (data == 0)
    {
        printf("pass");
    }
    else
    {
        printf("fail");
    }
#endif

#if TEST==6 //pass
    char str6[] = {SLAVE_ID, 0x06, 0x00, 0x00, 0x12, 0x34, 0x39, 0x85};
    usart_send(str6, sizeof(str6)/sizeof(char));
    sleep(2);
    mdU16 data;
    mdhandler->registerPool->mdReadU16(mdhandler->registerPool, 0 + HOLD_REGISTER_OFFSET, &data);
    if (data == 0x1234)
    {
        printf("pass");
    }
    else
    {
        printf("fail");
    }
#endif

#if TEST==15    //pass
    char str15[] = {SLAVE_ID, 0x0f, 0x00, 0x00, 0x00, 0x0a, 0x02, 0xcd, 0x01, 0xa8, 0x42};
    usart_send(str15, sizeof(str15)/sizeof(char));
    sleep(2);
    mdU16 data;
    mdhandler->registerPool->mdReadU16(mdhandler->registerPool, 0 + COIL_OFFSET, &data);
    if (data == 1)
    {
        printf("pass");
    }
    else
    {
        printf("fail");
    }
#endif

#if TEST==16    //pass
    char str16[] = {SLAVE_ID, 0x10, 0x00, 0x00, 0x00, 0x02, 0x04, 0x12, 0x34, 0x56, 0x78, 0xab, 0x91};
    usart_send(str16, sizeof(str16)/sizeof(char));
    sleep(2);
    mdU16 data;
    mdhandler->registerPool->mdReadU16(mdhandler->registerPool, 1 + HOLD_REGISTER_OFFSET, &data);
    if (data == 0x5678)
    {
        printf("pass");
    }
    else
    {
        printf("fail data=%X", data);
    }
#endif
}

void initTestCode()
{
    RegisterPoolHandle handler = mdhandler->registerPool;
    for (mdU32 i = 0; i < 100; i++)
    {
        handler->mdWriteCoil(handler, i, mdHigh);
    }
        for (mdU32 i = 0; i < 100; i++)
    {
        handler->mdWriteInputCoil(handler, i, mdHigh);
    }
        for (mdU32 i = 0; i < 1; i++)
    {
        handler->mdWriteHoldRegister(handler, i, 0x1234);
    }
        for (mdU32 i = 0; i < 100; i++)
    {
        handler->mdWriteInputRegister(handler, i, 0x5678);
    }
}

struct StartTimer st;

int main()
{
    int ret;
    char c;
    struct ModbusRTUSlaveRegisterInfo info;
    info.slaveId = SLAVE_ID;
    info.usartBaudRate = BUAD_RATE;
    info.mdRTUPopChar = popchar;
    mdCreateModbusRTUSlave(&mdhandler,info);

    initTestCode();

    pthread_mutex_init(&mut,NULL);
    pthread_mutex_init(&mdmux,NULL);
    pthread_cond_init(&cond,NULL);

    st.utime = BYTE_UTIME;
    st.callback = timer_handler;
    if((ret = CreateTimer(&timer, &st)) != 0)
    {
        printf("创建定时器失败.");
    }

    while (1)
    {
        sendtest();
    }

    pthread_mutex_lock(&mut);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mut);

    pthread_join(timer,NULL);

    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mut);
    pthread_mutex_destroy(&mdmux);
    return 0;
}