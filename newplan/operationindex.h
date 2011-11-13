#pragma once

#ifndef NO_ASSOCIATIONS
#include <BWAPI.h>
#endif

#include <string>

class OperationIndex;
class AllOperationIndices;

class OperationIndex : boost::totally_ordered<ResourceIndex>, boost::incrementable<ResourceIndex>
{
	typedef OperationIndex ThisType;
	
	public:
		// (Auto-generated file!) Defines additionally: IndexBegin, IndexEnd (end ^= last + 1)
		#include "operationenum.h"
		
		OperationIndex() : index_(None)
		{ }
		
		OperationIndex(type i) : index_(i)
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

class AllOperationIndices
{
	public:
		int size() const
		{
			return OperationIndex::IndexEnd;
		}

		IndexType begin() const
		{
			return OperationIndex(OperationIndex::IndexFirst);
		}

		IndexType end() const
		{
			return OperationIndex(OperationIndex::IndexEnd);
		}
};