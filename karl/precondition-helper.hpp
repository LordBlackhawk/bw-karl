#pragma once

#include "precondition.hpp"

struct PreconditionSorter
{
    bool operator () (const Precondition* p1, const Precondition* p2) const
    {
        return p1->time < p2->time;
    }
};

inline int calcMaxTime(Precondition* first)
{
    if (first == NULL)
        return 0;
    return first->time;
}

template <class ... List>
int calcMaxTime(Precondition* first, List... args)
{
    return std::max(calcMaxTime(first), calcMaxTime(args...));
}

inline void setWishTimeSimple(int wishtime, Precondition* pre)
{
    if (pre == NULL)
        return;
    
    pre->wishtime = wishtime;
}

template <class ... List>
void setWishTimeSimple(int wishtime, Precondition* first, List... args)
{
    setWishTimeSimple(wishtime, first);
    setWishTimeSimple(wishtime, args...);
}

template <class ... List>
bool updateTimePreconditions(Precondition* self, int duration, List... args)
{
    int time = calcMaxTime(args...);
    if (time == 0) {
        setWishTimeSimple(0, args...);
        return true;
    }
    int oldtime  = self->time - duration;
    int wishtime = self->wishtime - duration;
    self->time   = time + duration;

    int dt       = std::max(oldtime - time, 1);
    int subtime  = std::max(time - 10*dt, wishtime);
    setWishTimeSimple(subtime, args...);
    return false;
}

template <class T>
void release(T*& pre)
{
    if (pre != NULL) {
        delete pre;
        pre = NULL;
    }
}
