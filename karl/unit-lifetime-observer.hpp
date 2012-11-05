#pragma once

#include "unit-observer.hpp"

template <class Derived>
struct UnitLifetimeObserver
{
	typedef UnitLifetimeObserver<Derived>		ThisType;
	
	struct MyPrecondition : public UnitObserver<MyPrecondition>
	{
		ThisType* owner;
		
		MyPrecondition(ThisType* o, UnitPrecondition* p)
			: UnitObserver<MyPrecondition>(p), owner(o)
		{
			owner->pre = this;
		}
	
		void onFulfilled()
		{
			if (owner != NULL) {
				owner->unit = this->unit;
				owner->This()->onUnitReady();
			}
		}
	
		void onRemoveFromList()
		{
			if (owner != NULL)
				owner->pre = NULL;
		}
	};
	
	MyPrecondition* 	pre;
	BWAPI::Unit*		unit;
    bool                inUpdate;
	
	Derived* This()
	{
		return static_cast<Derived*>(this);
	}

	UnitLifetimeObserver(BWAPI::Unit* u)
		: pre(NULL), unit(u), inUpdate(false)
	{ }
	
	UnitLifetimeObserver(UnitPrecondition* p)
		: pre(new MyPrecondition(this, p)), unit(NULL), inUpdate(false)
	{ }
	
	static UnitPrecondition* createObserver(UnitPrecondition* p)
	{
		Derived* result = new Derived(p);
		return result->pre;
	}
	
	~UnitLifetimeObserver()
	{
        if (!inUpdate)
            This()->onRemoveFromList();
	}
	
	bool update()
	{
        inUpdate = true;
		if (pre != NULL)
			if (pre->update())
                pre = NULL;
		
		if (unit != NULL) {
			if (!This()->isAlive()) {
				This()->onUnitDestroyed();
                inUpdate = false;
				return true;
			}
			This()->onUpdateOnline();
		} else if (pre != NULL) {
			This()->onUpdateOffline();
		} else {
            inUpdate = false;
			return true;
		}
        inUpdate = false;
		return false;
	}
	
	/* Methods to overwrite: */
	void onRemoveFromList()
	{ }
	
	void onUnitDestroyed()
	{ }
	
	void onUnitReady()
	{ }
	
	void onUpdateOnline()
	{ }
	
	void onUpdateOffline()
	{ }
	
	bool isAlive() const
	{
		return unit->exists();
	}
};
