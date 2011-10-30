#pragma once

#include "typelists.h"

template <class RT_, int f, class DepRT_>
struct LinearGrowth
{
	typedef RT_			RT;
	typedef DepRT_		depRT;
	enum { factor = f };
};

namespace Plan
{
	template <class RT>
	struct ResourceName
	{
		static const char name[];
	};
   
	template <class RT>
	struct ResourceLockable
	{
		enum { value = false };
	};
  
	template <class RT>
	struct ResourceGrowth
	{
		enum { value = false };
		enum { scaling = 1 };
		typedef TL::type_list<> type;
	};
}
 
#define DEF_RESTYPE(Name)                                                      \
	struct Name { };                                                           \
	template <> const char Plan::ResourceName<Name>::name[] = #Name;

#define DEF_RESLOCKABLE(Name)                                                  \
  struct Name;                                                                 \
  namespace Plan { template <> struct ResourceLockable<Name>                   \
  { enum { value = true }; }; }
  
#define BEGIN_DEF_RESGROWTH(Name, Scaling)										\
	struct Name;																\
	namespace Plan { template <> struct ResourceGrowth<Name>					\
	{   enum { value = true };													\
		enum { scaling = Scaling };												\
		typedef Name InternalRT;												\
		typedef TL::type_list<

#define LINEAR(Num, DepName)													\
	LinearGrowth<InternalRT, Num, DepName>
		
#define END_DEF_RESGROWTH														\
			> type;																\
	}; }

template <class RLIST>
struct GrowthPairs
{
	typedef TL::type_list<> type;
};

template <class F, class ... T>
struct GrowthPairs< TL::type_list<F, T...> >
{
	typedef typename TL::combine<
							typename Plan::ResourceGrowth<F>::type,
							typename GrowthPairs< TL::type_list<T...> >::type
						>::type type;
};

template <class RLIST, class depRT>
struct GrowthInverseList
{
	template <class PT>
	struct Predicate
	{
		enum { value = boost::is_same<depRT, typename PT::depRT>::value };
	};
  
	typedef typename GrowthPairs<RLIST>::type list;
	typedef typename TL::sublist< Predicate, list >::type type;
};

