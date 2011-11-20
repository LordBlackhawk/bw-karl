#pragma once

#include "resources.h"
#include "operations.h"
#include "checkpoints.h"
#include "plan.h"
#include "add-linear-correction.h"
#include "fallbackbehaviour.h"

namespace {

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
	if (res.get(ri) < num)
		result = res.firstMoreThan(ri, num, blocking);
}

/* for Operation::apply(): */
void Locks(Resources& res, int num, const ResourceIndex& ri, const TimeInterval& interval, const TimeType& applytime, bool pushdecs)
{
	if (interval.contains(applytime, pushdecs))
		res.incLocked(ri, applytime, num);
}

void Unlocks(Resources& res, int num, const ResourceIndex& ri, const TimeInterval& interval, const TimeType& applytime, bool /*pushdecs*/)
{
	if (interval.contains(applytime))
		res.decLocked(ri, applytime, num);
}

void Consums(Resources& res, int num, const ResourceIndex& ri, const TimeInterval& interval, const TimeType& applytime, bool pushdecs)
{
	if (interval.contains(applytime, pushdecs))
		res.dec(ri, applytime, num);
}

void Prods(Resources& res, int num, const ResourceIndex& ri, const TimeInterval& interval, const TimeType& applytime, bool /*pushdecs*/)
{
	if (interval.contains(applytime))
		res.inc(ri, applytime, num);
}

} // end namespace

bool ResourceIndex::isLockable() const
{
	return (index_ < IndexLockedEnd);
}

ResourceIndex ResourceIndex::byName(const std::string& name)
{
	typedef std::map<std::string, ResourceIndex> MapType;
	static MapType fast = []
		{
			MapType result;
			for (auto it : AllResourceIndices())
				result[it.getName()] = it;
			return result;
		}();

	auto it = fast.find(name);
	if (it == fast.end())
		return ResourceIndex::None;
	return it->second;
}

ResourceIndex ResourceIndex::byUnitType(const BWAPI::UnitType& ut)
{
	typedef std::map<BWAPI::UnitType, ResourceIndex> MapType;
	static MapType fast = []
		{
			MapType result;
			for (auto it : AllResourceIndices()) {
				BWAPI::UnitType t = it.associatedUnitType();
				if (t != BWAPI::UnitTypes::None)
					result[t] = it;
			}
			return result;
		}();

	auto it = fast.find(ut);
	if (it == fast.end())
		return ResourceIndex::None;
	return it->second;
}

ResourceIndex ResourceIndex::byTechType(const BWAPI::TechType& tt)
{
	typedef std::map<BWAPI::TechType, ResourceIndex> MapType;
	static MapType fast = []
		{
			MapType result;
			for (auto it : AllResourceIndices()) {
				BWAPI::TechType t = it.associatedTechType();
				if (t != BWAPI::TechTypes::None)
					result[t] = it;
			}
			return result;
		}();

	auto it = fast.find(tt);
	if (it == fast.end())
		return ResourceIndex::None;
	return it->second;
}

ResourceIndex ResourceIndex::byUpgradeType(const BWAPI::UpgradeType& gt)
{
	typedef std::map<BWAPI::UpgradeType, ResourceIndex> MapType;
	static MapType fast = []
		{
			MapType result;
			for (auto it : AllResourceIndices()) {
				BWAPI::UpgradeType t = it.associatedUpgradeType();
				if (t != BWAPI::UpgradeTypes::None)
					result[t] = it;
			}
			return result;
		}();

	auto it = fast.find(gt);
	if (it == fast.end())
		return ResourceIndex::None;
	return it->second;
}

OperationIndex OperationIndex::byName(const std::string& name)
{
	typedef std::map<std::string, OperationIndex> MapType;
	static MapType fast = []
		{
			MapType result;
			for (auto it : AllOperationIndices())
				result[it.getName()] = it;
			return result;
		}();

	auto it = fast.find(name);
	if (it == fast.end())
		return OperationIndex::None;
	return it->second;
}

OperationIndex OperationIndex::byUnitType(const BWAPI::UnitType& ut)
{
	typedef std::map<BWAPI::UnitType, OperationIndex> MapType;
	static MapType fast = []
		{
			MapType result;
			for (auto it : AllOperationIndices()) {
				BWAPI::UnitType t = it.associatedUnitType();
				if (t != BWAPI::UnitTypes::None)
					result[t] = it;
			}
			return result;
		}();

	auto it = fast.find(ut);
	if (it == fast.end())
		return OperationIndex::None;
	return it->second;
}

OperationIndex OperationIndex::byTechType(const BWAPI::TechType& tt)
{
	typedef std::map<BWAPI::TechType, OperationIndex> MapType;
	static MapType fast = []
		{
			MapType result;
			for (auto it : AllOperationIndices()) {
				BWAPI::TechType t = it.associatedTechType();
				if (t != BWAPI::TechTypes::None)
					result[t] = it;
			}
			return result;
		}();

	auto it = fast.find(tt);
	if (it == fast.end())
		return OperationIndex::None;
	return it->second;
}

OperationIndex OperationIndex::byUpgradeType(const BWAPI::UpgradeType& gt)
{
	typedef std::map<BWAPI::UpgradeType, OperationIndex> MapType;
	static MapType fast = []
		{
			MapType result;
			for (auto it : AllOperationIndices()) {
				BWAPI::UpgradeType t = it.associatedUpgradeType();
				if (t != BWAPI::UpgradeTypes::None)
					result[t] = it;
			}
			return result;
		}();

	auto it = fast.find(gt);
	if (it == fast.end())
		return OperationIndex::None;
	return it->second;
}

FallbackBehaviourType::type PlanContainer::push_back_sr(const Operation& op)
{
	DefaultFallbackBehaviour dfbb;
	SimpleFallbackBehaviour<DefaultFallbackBehaviour> sfbb(dfbb);
	return push_back(op, sfbb);
}

FallbackBehaviourType::type PlanContainer::push_back_df(const Operation& op)
{
	DefaultFallbackBehaviour dfbb;
	return push_back(op, dfbb);
}

bool PlanContainer::rebase_sr(TimeType timeinc, const Resources& newres)
{
	DefaultFallbackBehaviour dfbb;
	SimpleFallbackBehaviour<DefaultFallbackBehaviour> sfbb(dfbb);
	return rebase(timeinc, newres, sfbb);
}

bool PlanContainer::rebase_df(TimeType timeinc, const Resources& newres)
{
	DefaultFallbackBehaviour dfbb;
	return rebase(timeinc, newres, dfbb);
}