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
				owner->onUnitReady();
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
	
	Derived* This()
	{
		return static_cast<Derived*>(this);
	}

	UnitLifetimeObserver(BWAPI::Unit* u)
		: pre(NULL), unit(u)
	{ }
	
	UnitLifetimeObserver(UnitPrecondition* p)
		: pre(new MyPrecondition(this, p)), unit(NULL)
	{ }
	
	static UnitPrecondition* createObserver(UnitPrecondition* p)
	{
		Derived* result = new Derived(p);
		return result->pre;
	}
	
	~UnitLifetimeObserver()
	{
		This()->onRemoveFromList();
	}
	
	bool update()
	{
		if (pre != NULL)
			pre->update();
		
		if (unit != NULL) {
			if (!This()->isAlive()) {
				This()->onUnitDestroyed();
				return true;
			}
			This()->onUpdateOnline();
		} else if (pre != NULL) {
			This()->onUpdateOffline();
		} else {
			return true;
		}
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
