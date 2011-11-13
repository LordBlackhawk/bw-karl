#pragma once

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
		static ThisType byName(const std::string& name)
		{
			for (auto it : AllOperationIndices())
				if (it->getName() == name)
					return *it;
			return OperationIndex(None);
		}		
		
		bool valid() const
		{
			return (index_ >= 0) && (index_ < IndexEnd);
		}

		int getIndex() const
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
		
		/*
		template <class T>
		bool hasDetails() const
		{
			return HasDetails<T>::call();
		}
		*/

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