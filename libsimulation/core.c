
#include "core.h"



static void *timer_handler(void *arg)
{
    struct StartTimer *st = (struct StartTimer *)arg;
    while (1)
    {
        st->callback();
        usleep(st->utime);
    }
}

int CreateTimer(ptimer* p, struct StartTimer *st)
{
    int ret;
    if((ret = pthread_create(p, NULL, timer_handler, st)) != 0)
    {
        return -1;
    }
    return 0;
}