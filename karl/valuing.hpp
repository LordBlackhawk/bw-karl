#pragma once

#include <BWTA.h>

double valueLarvaAssignment(int time, int wishtime, bool assigned);
double valueResources(int time, int wishtime, int index);
double valueExpo(BWTA::BaseLocation* expo, BWTA::BaseLocation* home);
