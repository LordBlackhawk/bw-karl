#pragma once

#ifndef NO_ASSOCIATIONS
#include <BWAPI.h>
#endif

#include <boost/operators.hpp>
#include <string>

class ResourceIndex;
class AllResourceIndices;
class LockedResourceIndices;

class OperationIndex;

class ResourceIndex : boost::totally_ordered<ResourceIndex>, boost::incrementable<ResourceIndex>
{
	typedef ResourceIndex ThisType;

	public:
		// (Auto-generated file!) Defines additionally: IndexBegin, IndexLockedEnd, IndexEnd (end ^= last + 1)
		#include "resourceenum.h"
		
		ResourceIndex() : index_(None)
		{ }
		
		ResourceIndex(type i) : index_(i)
		{ }

		// Auto-generated functions see bwplan.cpp
		std::string getName() const;
		static ThisType byName(const std::string& name);
		
	#ifndef NO_ASSOCIATIONS
		BWAPI::Race associatedRace() const;
		BWAPI::UnitType associatedUnitType() const;
		BWAPI::TechType associatedTechType() const;
		BWAPI::UpgradeType associatedUpgradeType() const;
		
		static ThisType byUnitType(const BWAPI::UnitType& ut);
		static ThisType byTechType(const BWAPI::TechType& tt);
		static ThisType byUpgradeType(const BWAPI::UpgradeType& gt);
	#endif
		
		std::set<OperationIndex> getAssociatedOperations() const;
		
		bool isLockable() const;
		bool isGrowthing() const;
		int getScaling() const;
		
		bool valid() const
		{
			return (index_ >= 0) && (index_ < IndexEnd);
		}
		
		int getIndex() const
		{
			return index_;
		}
		
		type getType() const
		{
			return index_;
		}

		ThisType& operator ++ ()
		{
			++index_;
			return *this;
		}
		
		bool operator == (const ThisType& other) const
		{
			return index_ == other.index_;
		}
		
		bool operator < (const ThisType& other) const
		{
			return index_ < other.index_;
		}

	protected:
		type index_;
};

class AllResourceIndices
{
	public:
		int size() const
		{
			return ResourceIndex::IndexEnd;
		}

		IndexType begin() const
		{
			return ResourceIndex(ResourceIndex::IndexBegin);
		}

		IndexType end() const
		{
			return ResourceIndex(ResourceIndex::IndexEnd);
		}
};

class LockedResourceIndices
{
	public:
		int size() const
		{
			return ResourceIndex::IndexLockedEnd;
		}

		IndexType begin() const
		{
			return ResourceIndex(ResourceIndex::IndexBegin);
		}

		IndexType end() const
		{
			return ResourceIndex(ResourceIndex::IndexLockedEnd);
		}
};
