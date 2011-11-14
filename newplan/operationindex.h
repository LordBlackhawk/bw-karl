#pragma once

#ifndef NO_ASSOCIATIONS
#include <BWAPI.h>
#endif

#include <string>

class OperationIndex;
class AllOperationIndices;

class OperationIndex : boost::totally_ordered<OperationIndex>, boost::incrementable<OperationIndex>
{
	typedef OperationIndex ThisType;
	
	public:
		// (Auto-generated file!) Defines additionally: IndexBegin, IndexEnd (end ^= last + 1)
		#include "operationenum.h"
		
		OperationIndex() : index_(None)
		{ }
		
		OperationIndex(Type i) : index_(i)
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
		
		Type getType() const
		{
			return index_;
		}
		
		ThisType& operator * ()
		{
			return *this;
		}
		
		const ThisType& operator * () const
		{
			return *this;
		}

		ThisType& operator ++ ()
		{
			index_ = Type(int(index_)+1);
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
		Type index_;
};

class AllOperationIndices
{
	public:
		int size() const
		{
			return OperationIndex::IndexEnd;
		}

		OperationIndex begin() const
		{
			return OperationIndex(OperationIndex::IndexBegin);
		}

		OperationIndex end() const
		{
			return OperationIndex(OperationIndex::IndexEnd);
		}
};