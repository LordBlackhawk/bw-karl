#pragma once

#include "resources.h"
#include "operations.h"
#include "checkpoints.h"
#include "add-linearcorrection.h"

namespace {

/* for Operation::execute(): */
template <class FUNC>
void Call(const FUNC& f, Operation& op)
{
	CheckPointResult::type res = f(op);
	switch (res)
	{
	case CheckPointResult::waiting:
		break;
		
	case CheckPointResult::running:
		op.status = OperationStatus::running;
		break;
		
	case CheckPointResult::completed:
		op.status = OperationStatus::completed;
		break;
	
	case CheckPointResult::failed:
		op.status = OperationStatus::failed;
		break;
	}
}

/* for Operation::firstApplyableAt(): */
void Needs(const Resources& res, int num, const ResourceIndex& ri, TimeType& result, ResourceIndex& blocking)
{
	if (res.getExisting(ri) < num) {
		blocking = ri;
		result   = std::numeric_limits<TimeType>::max();
	}
}

void Locks(const Resources& res, int num, const ResourceIndex& ri, TimeType& result, ResourceIndex& blocking)
{
	if (res.get(ri) < num) {
		blocking = ri;
		result   = std::numeric_limits<TimeType>::max();
	}
}

void Consums(const Resources& res, int num, const ResourceIndex& ri, TimeType& result, ResourceIndex& blocking)
{
	if (res.get(ri) < num) {
		blocking = ri;
		switch (ri.getType())
		{
			case ResourceIndex::Minerals:
			case ResourceIndex::Gas:
			case ResourceIndex::Larva:
				int growth = res.getGrowth(ri);
				int value  = res.getInternalValue(ri);
				result = (num - value + growth-1)/growth;
				break;
			default:
				result = std::numeric_limits<TimeType>::max();
				break;
		}
	}
}

/* for Operation::apply(): */
void Locks(Resources& res, int num, const ResourceIndex& ri, const TimeInterval& interval, const TimeType& applytime, bool pushdecs)
{
	if (interval.contains(applytime, pushdecs))
		res.incLocked(ri, applytime, num);
}

void Unlocks(Resources& res, int num, const ResourceIndex& ri, const TimeInterval& interval, const TimeType& applytime, bool pushdecs)
{
	if (interval.contains(applytime))
		res.decLocked(ri, applytime, num);
}

void Consums(Resources& res, int num, const ResourceIndex& ri, const TimeInterval& interval, const TimeType& applytime, bool pushdecs)
{
	if (interval.contains(applytime, pushdecs))
		res.dec(ri, applytime, num);
}

void Prods(Resources& res, int num, const ResourceIndex& ri, const TimeInterval& interval, const TimeType& applytime, bool pushdecs)
{
	if (interval.contains(applytime))
		res.inc(ri, applytime, num);
}

} // end namespace

bool ResourceIndex::isLockable() const
{
	return (index_ < IndexLockedEnd);
}

static ResourceIndex ResourceIndex::byName(const std::string& name)
{
	typedef std::map<std::string, ResourceIndex> MapType;
	static MapType fast = []
		{
			MapType result;
			for (auto it : AllResourceIndices())
				result[it.getName()] = it;
			return result;
		};

	auto it = fast.find(name);
	if (it == fast.end())
		return ResourceIndex::None;
	return *it;
}

static OperationIndex OperationIndex::byName(const std::string& name)
{
	typedef std::map<std::string, OperationIndex> MapType;
	static MapType fast = []
		{
			MapType result;
			for (auto it : AllOperationIndices())
				result[it.getName()] = it;
			return result;
		};

	auto it = fast.find(name);
	if (it == fast.end())
		return OperationIndex::None;
	return *it;
}

// TODO: by...