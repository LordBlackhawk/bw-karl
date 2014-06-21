#pragma once

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
};

template <class Derived>
int ObjectCounter<Derived>::objectsAlive = 0;
