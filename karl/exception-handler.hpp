#pragma once

#include "default-code.hpp"

struct ExceptionHandlerCode : public DefaultCode
{
    static void onProgramStart(const char* programname);
};
