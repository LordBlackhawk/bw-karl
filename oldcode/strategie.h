#ifndef STRATEGIE_h
#define STRATEGIE_h

#include "task.h"

class Strategie
{
public:
	Task* toptask;

public:
	Strategie(Task* tt)
		: toptask(tt)
	{ }

public:
	virtual void startup() { }
	virtual void computeActions(int fcount) { }
	virtual void taskChangedStatus(Task* task) { }
};

#endif