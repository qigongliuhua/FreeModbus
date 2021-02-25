
#include "core.h"

struct StartTimer
{
    func callback;
    size_t utime;
};

static void *timer_handler(void *arg)
{
    struct StartTimer *st = (struct StartTimer *)arg;
    while (1)
    {
        st->callback();
        usleep(st->utime);
    }
}

int CreateTimer(ptimer* p, size_t utime, func callback)
{
    int ret;
    struct StartTimer st;
    st.utime = utime;
    st.callback = callback;
    if((ret = pthread_create(p, NULL, timer_handler, &st)) != 0)
    {
        return -1;
    }
    return 0;
}