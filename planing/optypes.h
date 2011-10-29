#pragma once

#include "typelists.h"

template <int, class RT>
struct Needs
{ };

template <int, class RT>
struct Locks
{ };

template <int, class RT>
struct Unlocks
{ };

template <int, class RT>
struct Consums
{ };

template <int, class RT>
struct Prods
{ };

template <class HT, int = 1>
struct CheckPoint
{ };

namespace Plan
{
	template <class OT>
	struct OperationName
	{
		static const char name[];
	};
	
	template <class OT>
	struct OperationList
	{ };
   
	template <class OT>
	struct OperationDetailType
	{
		typedef int type;
	};
  
	template <class OT, class DT>
	struct OperationDynamic
	{ };
 
	template <class OT, class HT, int num>
	struct OperationDynamic<OT, CheckPoint<HT, num> >
	{
		static int getValue(int* /*details*/) { return num; }
	};
}

#define BEGIN_DEF_OPTYPE(Name)														\
	struct Name { };                                   							  	\
	template <> const char Plan::OperationName<Name>::name[] = #Name;				\
	namespace Plan { template <> struct OperationList<Name> {		   				\
		typedef TL::type_list<
		
#define END_DEF_OPTYPE															   \
		> type;																       \
	}; }

#define DEF_OPDETAILS(Name, Type)                                                  	\
  struct Name;																		\
  namespace Plan { template <> struct OperationDetailType<Name>        				\
  { typedef Type type; }; }
  
#define DEF_OPDYNDURATION(Name, Num)                                               						\
  struct Name;																							\
  namespace Plan { template <class HT> struct OperationDynamic<Name, CheckPoint<HT, Num> >				\
  {                                                                               						\
    enum { defaultValue = Num };                                                   						\
    typedef OperationDetailType<Name>::type Details;                   									\
    static int getValueInternal(const Details& details);                           						\
    static inline int getValue(int* d)                                             						\
    { Details* details = (Details*) d; return getValueInternal(*details); }        						\
  };  }                                                                            						\
  template <class HT> int Plan::OperationDynamic<Name, CheckPoint<HT, Num> >							\
	::getValueInternal(const Details& details)

