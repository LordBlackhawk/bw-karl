#include "timer.hpp"
#include "average-calculator.hpp"
#include "log.hpp"
#include <windows.h>

namespace
{
	LARGE_INTEGER counter_start, counter_end;
	AverageCalculator<100, int, double> avg;
}

void timerInit()
{
	LARGE_INTEGER frequency;
	if (!QueryPerformanceFrequency(&frequency))
		LOG << "No high resolution timer!";
	avg.setFactor(1000000.0 / double(frequency.QuadPart));	
}

void timerStart()
{
	QueryPerformanceCounter(&counter_start);
}

void timerEnd()
{
	QueryPerformanceCounter(&counter_end);
	avg.push(counter_end.QuadPart - counter_start.QuadPart);
}

double timerMin()
{
	return avg.min();
}

double timerAverage()
{
	return avg.average();
}

double timerMax()
{
	return avg.max();
}
