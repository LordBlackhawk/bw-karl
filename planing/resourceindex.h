#pragma once

#include "associations.h"
#include "restypes.h"

template <class RLIST>
class ResourceIndex
{
	typedef ResourceIndex<RLIST>											ThisType;
	typedef typename TL::sublist< Plan::ResourceLockable, RLIST >::type		LOCKLIST;

	public:
		template <class RT>
		static ThisType byClass()
		{
			return ResourceIndex(TL::indexof<RT, RLIST>::value);
		}

		template <class T>
		static ThisType byAssociation(const T& value)
		{
			int result = -1;
			TL::enumerate<RLIST>::template call<ByAssociation, const T&, int&>(value, result);
			return ResourceIndex(result);
		}

		static ThisType byName(const std::string& name)
		{
			int result = -1;
			TL::enumerate<RLIST>::template call<ByName, const std::string&, int&>(name, result);
			return ResourceIndex(result);
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

		int getIndexLocked() const
		{
			int result = -1;
			TL::dispatch<RLIST>::template call<GetIndexLocked, int&>(index_, result);
			return result;
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

	protected:
		int index_;

		ResourceIndex(int i) : index_(i)
		{ }
};

template <class RLIST>
class ResourceIndexContainer
{
	public:
		int size() const
		{
			return TL::size<RLIST>::value;
		}

		ResourceIndex<RLIST> begin() const
		{
			return ResourceIndex<RLIST>(0);
		}

		ResourceIndex<RLIST> end() const
		{
			return ResourceIndex<RLIST>(size());
		}
};

ResourceIndexContainer ResourceIndices;