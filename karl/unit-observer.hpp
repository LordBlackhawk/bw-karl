#pragma once

#include "bwapi-precondition.hpp"
#include "precondition-helper.hpp"

template <class Derived>
struct UnitObserver : public UnitPrecondition
{
	UnitPrecondition* pre;
	
	UnitObserver(UnitPrecondition* p)
		: UnitPrecondition(p->time, p->ut, p->pos), pre(p)
	{ }
	
	~UnitObserver()
	{
		release(pre);
		This()->onRemoveFromList();
	}
	
	Derived* This()
	{
		return static_cast<Derived*>(this);
	}
	
	bool update()
	{
		assert(pre != NULL);
		time          = pre->time;
		pre->wishtime = wishtime;
		if (isFulfilled()) {
			unit = pre->unit;
			release(pre);
			This()->onFulfilled();
			return true;
		} else {
			return false;
		}
	}
	
	/* Methods to overwrite: */
	void onFulfilled()
	{ }
	
	void onRemoveFromList()
	{ }
};
