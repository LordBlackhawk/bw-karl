#pragma once

#include "myseh.hpp"
#include "addr2line.hpp"
#include <iomanip>

#define SEH_MAGIC_NUMBER 0xDEADBEEF

extern "C" {
    seh::Registrar* __stdcall __libseh_get_registration();
    void __stdcall __libseh_set_registration(volatile seh::Registrar* reg);
    seh::Registrar* __stdcall __libseh_pop_registration();
    void __stdcall __libseh_unwind_to(void* stackpointer);
}
