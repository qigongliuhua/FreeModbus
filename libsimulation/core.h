#ifndef _CORE_H_
#define _CORE_H_

#include <pthread.h>
#include <unistd.h>

typedef pthread_t ptimer;

typedef void(*func)(void);

struct StartTimer
{
    func callback;
    size_t utime;
};

extern int CreateTimer(ptimer *p, struct StartTimer *st);

#endif