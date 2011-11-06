#pragma once

#include "association.h"
#include "restypes.h"

#include <string>

template <class Traits>
class ResourceIndexContainer;

template <class Traits>
class ResourceIndex
{
	typedef typename Traits::ResourceList				RLIST;
	friend class ResourceIndexContainer<Traits>;

	public:
		typedef ResourceIndex<Traits>											ThisType;
		typedef typename TL::sublist< Plan::ResourceLockable, RLIST >::type		LOCKLIST;
		typedef ResourceIndexContainer<Traits>									ContainerType;

	public:
		enum { IndexCount = TL::size<RLIST>::value };
		enum { LockedIndexCount = TL::size<LOCKLIST>::value };
	
		template <class RT>
		static ThisType byClass()
		{
			return ThisType(TL::indexof<RT, RLIST>::value);
		}

		template <class T>
		static ThisType byAssociation(const T& value)
		{
			int result = -1;
			TL::enumerate<RLIST>::template call<ByAssociation, const T&, int&>(value, result);
			return ThisType(result);
		}

		static ThisType byName(const std::string& name)
		{
			int result = -1;
			TL::enumerate<RLIST>::template call<ByName, const std::string&, int&>(name, result);
			return ThisType(result);
		}
		
		static ThisType byUserName(const std::string& name)
		{
			return Traits::NameTraits::getResourceIndexByName(name);
		}
		
		ResourceIndex() : index_(-1)
		{ }
		
		bool valid() const
		{
			return (index_ >= 0) && (index_ < IndexCount);
		}

		std::string getName() const
		{
			std::string result = "[unknown]";
			TL::dispatch<RLIST>::template call<GetName, std::string&>(index_, result);
			return result;
		}

		bool isLockable() const
		{
			bool result = false;
			TL::dispatch<RLIST>::template call<IsLockable, bool&>(index_, result);
			return result;
		}

		int getIndex() const
		{
			return index_;
		}

		int getLockedIndex() const
		{
			int result = -1;
			TL::dispatch<RLIST>::template call<GetIndexLocked, int&>(index_, result);
			return result;
		}
		
		int getScaling() const
		{
			int result = -1;
			TL::dispatch<RLIST>::template call<GetScaling, int&>(index_, result);
			return result;
		}

		ThisType& operator * ()
		{
			return *this;
		}

		ThisType& operator -> ()
		{
			return *this;
		}

		ThisType& operator ++ ()
		{
			++index_;
			return *this;
		}
		
		bool operator != (const ThisType& other) const
		{
			return index_ != other.index_;
		}
		
		bool operator == (const ThisType& other) const
		{
			return index_ == other.index_;
		}
		
		bool operator < (const ThisType& other) const
		{
			return index_ < other.index_;
		}
		
		bool operator <= (const ThisType& other) const
		{
			return index_ <= other.index_;
		}
		
		bool operator > (const ThisType& other) const
		{
			return index_ < other.index_;
		}
		
		bool operator >= (const ThisType& other) const
		{
			return index_ <= other.index_;
		}

	private:
		template <class RT>
		struct ByAssociation
		{
			template <class T>
			static void call(const T& value, int& result)
			{
				if (getAssociation<RT, T>() == value)
					result = TL::indexof<RT, RLIST>::value;
			}
		};

		template <class RT>
		struct ByName
		{
			static void call(const std::string& name, int& result)
			{
				if (name == Plan::ResourceName<RT>::name)
					result = TL::indexof<RT, RLIST>::value;
			}
		};

		template <class RT>
		struct GetName
		{
			static void call(std::string& result)
			{
				result = Plan::ResourceName<RT>::name;
			}
		};

		template <class RT>
		struct IsLockable
		{
			static void call(bool& result)
			{
				result = Plan::ResourceLockable<RT>::value;
			}
		};

		template <class RT>
		struct GetIndexLocked
		{
			static void call(int& result)
			{
				result = TL::indexof<RT, LOCKLIST>::value;
			}
		};
		
		template <class RT>
		struct GetScaling
		{
			static void call(int& result)
			{
				result = Plan::ResourceGrowth<RT>::scaling;
			}
		};

	protected:
		int index_;

		ResourceIndex(int i) : index_(i)
		{ }
};

template <class Traits>
class ResourceIndexContainer
{
	typedef ResourceIndex<Traits>	IndexType;

	public:
		enum { IndexCount = IndexType::IndexCount };
		enum { LockIndexCount = IndexType::LockedIndexCount };
	
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
