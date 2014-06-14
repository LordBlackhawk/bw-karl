#ifndef INTERFACE_h
#define INTERFACE_h

#include "task.h"

class Strategie;

class Interface : public Task
{
public:
	Strategie* strategie;

public:
	Interface();
	void drawTaskInfo() const;
	void onFrame();
	virtual void subtaskChangedStatus(Task* task);
	void onUnitCreate(BWAPI::Unit* unit);
	void onUnitDestroy(BWAPI::Unit* unit);
	void onUnitMorph(BWAPI::Unit* unit);
	void onUnitRenegade(BWAPI::Unit* unit);
	void onUnitDiscover(BWAPI::Unit* unit);
	void onUnitEvade(BWAPI::Unit* unit);
	void onUnitShow(BWAPI::Unit* unit);
	void onUnitHide(BWAPI::Unit* unit);
};

#endif