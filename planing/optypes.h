#pragma once

#include "typelists.h"

#include <boost/shared_ptr.hpp>

typedef boost::shared_ptr<void> DetailPointerType;

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

struct Dummy // Last Item in Operation!!!!
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
		static int getValue(const DetailPointerType& /*details*/) { return num; }
	};
}

#define BEGIN_DEF_OPTYPE(Name)														\
	struct Name { };                                   							  	\
	template <> const char Plan::OperationName<Name>::name[] = #Name;				\
	namespace Plan { template <> struct OperationList<Name> {		   				\
		typedef TL::type_list<
		
#define END_DEF_OPTYPE															   \
		Dummy > type;														       \
	}; }

#define DEF_OPDETAILS(Name, Type)                                                  	\
  struct Name;																		\
  namespace Plan { template <> struct OperationDetailType<Name>        				\
  { typedef Type type; }; }
  
template <class OT>
boost::shared_ptr<typename Plan::OperationDetailType<OT>::type> convertDetails(const DetailPointerType & details_)
{
	return boost::static_pointer_cast<typename Plan::OperationDetailType<OT>::type>(details_);
}
  
#define DEF_OPDYNDURATION(Name, Num)                                               						\
  struct Name;																							\
  namespace Plan { template <class HT> struct OperationDynamic<Name, CheckPoint<HT, Num> >				\
  {                                                                               						\
    enum { defaultValue = Num };                                                   						\
    typedef boost::static_pointer_cast<OperationDetailType<Name>::type> Details;   						\
    static int getValueInternal(const Details& details);                           						\
    static inline int getValue(const DetailPointerType& d)                        						\
    { return getValueInternal(convertDetails<Name>(details)); }        									\
  };  }                                                                            						\
  template <class HT> int Plan::OperationDynamic<Name, CheckPoint<HT, Num> >							\
	::getValueInternal(const Details& details)

