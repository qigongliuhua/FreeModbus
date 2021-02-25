
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "mdrtuslave.h"
#include "core.h"

#define BUAD_RATE 9600
#define SLAVE_ID 5

ptimer timer;
static ModbusRTUSlaveHandle mdhandle;
static pthread_mutex_t mut;
static pthread_cond_t cond;

static pthread_mutex_t mdmux;

static void timer_handler(void)
{
    int ret = 0;
    pthread_mutex_lock(&mdmux);
    mdhandle->portRTUTimerTick(mdhandle, (mdU32)(1000000/BUAD_RATE));
    pthread_mutex_unlock(&mdmux);
}


static mdVOID popchar(ModbusRTUSlaveHandle handle, mdU8 c)
{
    putc(c,stdout);
    fflush(stdout);
}


static void usart_send(char *buf, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        pthread_mutex_lock(&mdmux);
        mdhandle->portRTUPushChar(mdhandle,buf[i]);
        pthread_mutex_unlock(&mdmux);
        if (i==len-2)
        {
            usleep(3000);
        }
        else

            usleep((mdU32)(1000000/BUAD_RATE));
    }

}

static void sendtest()
{
    mdU8 str[] = "123465789\n";
    usart_send(str,strlen(str));
    sleep(2);
}

int main()
{
    int ret;
    char c;
    struct ModbusRTUSlaveRegisterInfo info;
    info.slaveId = SLAVE_ID;
    info.usartBaudRate = BUAD_RATE;
    info.mdRTUPopChar = popchar;
    mdCreateModbusRTUSlave(&mdhandle,info);

    pthread_mutex_init(&mut,NULL);
    pthread_mutex_init(&mdmux,NULL);
    pthread_cond_init(&cond,NULL);

    if((ret = CreateTimer(&timer,(mdU32)(1000000/BUAD_RATE),timer_handler)) != 0)
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