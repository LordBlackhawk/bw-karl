#pragma once

#include "log.hpp"
#include "system-helper.hpp"
#include <typeinfo>

template <class Derived>
struct ObjectCounter
{
    static int objectsAlive;
    
    ObjectCounter()
    {
        ++objectsAlive;
    }
    
    ~ObjectCounter()
    {
        --objectsAlive;
    }
    
    static void checkObjectsAlive(const char* name = typeid(Derived).name())
    {
        if (objectsAlive > 0)
            WARNING << objectsAlive << " instance of " << demangle(name) << " still alive.";
    }
};

template <class Derived>
int ObjectCounter<Derived>::objectsAlive = 0;
