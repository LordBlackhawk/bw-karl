#pragma once

#include "default-code.hpp"

struct HUDCode : public DefaultCode
{
    static void onTick();
    static void onPausedTick();
    static void onSendText(const std::string& text);
};
