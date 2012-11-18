#pragma once

#include "arrangement.hpp"

template <class Generator, class Condition, class Selector, class Driver>
class ArrangementImpl : public Arrangement
{
    public:
        ArrangementImpl()
        { }
        
        ~ArrangementImpl()
        { }
        
        void onTick()
        { }
};
