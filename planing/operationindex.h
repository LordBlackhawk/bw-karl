#pragma once

#include "association.h"
#include "optypes.h"

#include <string>

template <class OLIST>
class OperationIndexContainer;

template <class OLIST>
class OperationIndex
{
	typedef OperationIndex<OLIST>			ThisType;
	
	friend class OperationIndexContainer<OLIST>;
	
	public:
		enum { IndexCount = TL::size<OLIST>::value };
		
		template <class OT>
		static ThisType byClass()
		{
			return ThisType(TL::indexof<OT, OLIST>::value);
		}
		
		template <class T>
		static ThisType byAssociation(const T& value)
		{
			int result = -1;
			TL::enumerate<OLIST>::template call<ByAssociation, const T&, int&>(value, result);
			return ThisType(result);
		}

		static ThisType byName(const std::string& name)
		{
			int result = -1;
			TL::enumerate<OLIST>::template call<ByName, const std::string&, int&>(name, result);
			return ThisType(result);
		}
		
		std::string getName() const
		{
			std::string result = "[unknown]";
			TL::dispatch<OLIST>::template call<GetName, std::string&>(index_, result);
			return result;
		}
		
		bool valid() const
		{
			return (index_ >= 0) && (index_ < IndexCount);
		}

		int getIndex() const
		{
			return index_;
		}
		
		ThisType& operator * () const
		{
			return *this;
		}

		ThisType& operator -> () const
		{
			return *this;
		}

		ThisType& operator ++ ()
		{
			++index_;
			return *this;
		}

	private:
		template <class OT>
		struct ByAssociation
		{
			template <class T>
			static void call(const T& value, int& result)
			{
				if (getAssociation<OT, T>() == value)
					result = TL::indexof<OT, OLIST>::value;
			}
		};

		template <class OT>
		struct ByName
		{
			static void call(const std::string& name, int& result)
			{
				if (name == Plan::OperationName<OT>::name)
					result = TL::indexof<OT, OLIST>::value;
			}
		};

		template <class OT>
		struct GetName
		{
			static void call(std::string& result)
			{
				result = Plan::OperationName<OT>::name;
			}
		};

	protected:
		int index_;

		OperationIndex(int i) : index_(i)
		{ }
};

template <class OLIST>
class OperationIndexContainer
{
	typedef OperationIndex<OLIST>	IndexType;
	
	public:
		enum { IndexCount = IndexType::IndexCount };
		
		int size() const
		{
			return IndexCount;
		}

		IndexType begin() const
		{
			return IndexType(0);
		}

		IndexType end() const
		{
			return IndexType(size());
		}
};