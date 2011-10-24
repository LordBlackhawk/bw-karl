#ifndef STRATEGIEFACTORY_h
#define STRATEGIEACTORY_h

#include "strategie.h"

class StrategieFactory
{
  public:
    static Strategie* createStrategie(Task* toptask);
};

#endif
