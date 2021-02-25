#ifndef _CORE_H_
#define _CORE_H_

#include <pthread.h>
#include <unistd.h>

typedef pthread_t ptimer;

typedef void(*func)(void);

extern int CreateTimer(ptimer *p, size_t utime, func callback);

#endif