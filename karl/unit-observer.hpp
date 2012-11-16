#pragma once

#include "bwapi-precondition.hpp"
#include "precondition-helper.hpp"

template <class Derived>
struct UnitObserver : public UnitPrecondition
{
    UnitPrecondition* pre;
    
    UnitObserver(UnitPrecondition* p)
        : UnitPrecondition(p->time, p->ut, p->pos, p->mod), pre(p)
    {
        This()->onUpdate();
    }
    
    ~UnitObserver()
    {
        release(pre);
        This()->onRemoveFromList();
    }
    
    Derived* This()
    {
        return static_cast<Derived*>(this);
    }
    
    bool update()
    {
        assert(pre != NULL);
        time          = pre->time;
        pos           = pre->pos;
        pre->wishtime = wishtime;
        pre->wishpos  = wishpos;
        if (isFulfilled()) {
            unit = pre->unit;
            release(pre);
            This()->onFulfilled();
            return true;
        } else {
            This()->onUpdate();
            return false;
        }
    }
    
    /* Methods to overwrite: */
    void onUpdate()
    { }
    
    void onFulfilled()
    { }
    
    void onRemoveFromList()
    { }
};
