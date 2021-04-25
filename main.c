
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

static void sendtest()
{
    char str[] = {SLAVE_ID, 0x01, 0x00, 0x00, 0x00, 0x06, 0x8c, 0xbd};
    usart_send(str, sizeof(str)/sizeof(char));
    sleep(2);
}

void initTestCode()
{
    RegisterPoolHandle handler = mdhandler->registerPool;
    for (size_t i = 0; i < 100; i++)
    {
        handler->mdWriteBit(handler, i, mdHigh);
    }
    mdBit a;
    mdBit* b = &a;
    mdBit* c = b;
    for (size_t i = 0; i < 100; i++)
    {
        handler->mdReadBit(handler, i, c);
        printf("%d", *c);
    }
    printf("\n");
    mdBit d[16];
    handler->mdReadBits(handler,0, 16, d);
    for (int i = 0; i < 16; i++)
    {
        printf("%d", d[i]);
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