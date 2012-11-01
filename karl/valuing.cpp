#include "valuing.hpp"
#include <algorithm>

double valueLarvaAssignment(int time, int wishtime, bool assigned)
{
    double result = 0.0;

    if (!assigned)
        result += 100.0;

    int dt = time - wishtime;
    if (dt > 0)
        result += dt*dt;
    else
        result += 0.01 * dt*dt;

    return result;
}

double valueResources(int time, int wishtime, int index)
{
    return 1000.0 * index + std::max(time, wishtime);
}
