#pragma once

#include "typelists.h"

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
  
	template <class RT, class depRT>
	struct ResourceGrowth
	{
		enum { value = false };
	};
}
 
#define DEF_RESTYPE(Name)                                                      \
	struct Name { };                                                           \
	template <> const char Plan::ResourceName<Name>::name[] = #Name;

#define DEF_RESLOCKABLE(Name)                                                  \
  struct Name;                                                                 \
  namespace Plan { template <> struct ResourceLockable<Name>                   \
  { enum { value = true }; }; }

#define DEF_RESGROWTH(Name, Factor, DepName)                                   \
  struct Name;                                                                 \
  struct DepName;                                                              \
  namespace Plan { template <> struct ResourceGrowth<Name, DepName>            \
  {                                                                            \
    enum { value = true };                                                     \
    enum { factor = Factor };                                                  \
	typedef Name     RT;													   \
	typedef DepName  depRT;                                                    \
  }; }

template <class RLIST>
struct GrowthPairs
{
	template <class T>
	struct Predicate
	{
		enum { value = T::value };
	};
	
	typedef typename TL::tensorlist< Plan::ResourceGrowth, RLIST >::type  TENSORLIST;
	typedef typename TL::sublist< Predicate, TENSORLIST >::type type;
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

