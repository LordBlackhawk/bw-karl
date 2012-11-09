#pragma once

class HUDTextOutput
{
    friend class HUDCode;
    
    private:
        int line;
        
        HUDTextOutput()
            : line(0)
        { }
    
    public:
        void printf(const char* fmt, ...)
            __attribute__ ((format (printf, 2, 3)));
};
