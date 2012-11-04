#pragma once

#include "default-code.hpp"

#include <BWAPI.h>

#include <sstream>

bool logDisplay(const std::string&, int, const std::string&, int);
void logInternal(const std::string&, int, const std::string&, int, const std::string&);

class Log
{
    public:
        enum Type { Warning = 0, Important = 1, Debug = 2 };

        Log(const std::string& f, int l, const std::string& fn, int v)
            : file(f), line(l), functionname(fn), level(v)
        {
            display = logDisplay(file, line, functionname, level);
        }

        ~Log()
        {
            if (display)
                logInternal(file, line, functionname, level, stream.str());
        }

        template <class T>
        Log& operator << (const T& t)
        {
            if (display)
                stream << t;
            return *this;
        }

    private:
        bool display;
        std::string file;
        int line;
        std::string functionname;
        int level;
        std::stringstream stream;
};

struct LogEater
{
    template <class T>
    LogEater& operator << (const T& t) const
    {
        return *this;
    }
};

#define WARNING Log(__FILE__, __LINE__, __func__, Log::Warning)
#define NOLOG   LogEater()

#ifdef NDEBUG
    #define LOG     NOLOG
    #define DEBUG   NOLOG
#else
    #define LOG     Log(__FILE__, __LINE__, __func__, Log::Important)
    #define DEBUG   Log(__FILE__, __LINE__, __func__, Log::Debug)
#endif

std::ostream& operator << (std::ostream& stream, const BWAPI::Position& pos);
std::ostream& operator << (std::ostream& stream, const BWAPI::TilePosition& tp);
std::ostream& operator << (std::ostream& stream, const BWAPI::Player& player);
std::ostream& operator << (std::ostream& stream, const BWAPI::UnitType& ut);
std::ostream& operator << (std::ostream& stream, const BWAPI::TechType& tt);
std::ostream& operator << (std::ostream& stream, const BWAPI::UpgradeType& gt);
std::ostream& operator << (std::ostream& stream, const BWAPI::Error& err);
std::ostream& operator << (std::ostream& stream, const BWAPI::Race& race);

struct LogCode : public DefaultCode
{
    static void onMatchBegin();
    static void onReadParameter(int argc, const char* argv[], int& cur);
    static void onMatchEndMessage(bool winner);
};
