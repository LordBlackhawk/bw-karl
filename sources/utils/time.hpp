#pragma once

typedef int Time;

#define ACTIVE_TIME          -1
#define START_TIME            0
#define INFINITE_TIME   2000000

inline bool isImpossibleTime(Time time)
{
    return (time < INFINITE_TIME/2);
}
