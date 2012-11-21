#pragma once

#include "default-code.hpp"

struct MessageDispatchCode : public DefaultCode
{
    static void onMatchBegin();
    static void onTick();
};
