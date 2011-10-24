#pragma once

template <int, class RT>
struct Needs
{ };

template <int, class RT>
struct Locks
{ };

template <int, class RT>
struct Consums
{ };

template <int, class RT>
struct Prods
{ };

template <int>
struct Duration
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
    typedef void type;
  };
  
  template <class OT, class DT>
  struct OperationDynamic
  { };
 
  template <class OT, int num>
  struct OperationDynamic<OT, Duration<num> >
  {
    static int getValue(void* /*details*/) { return num; }
  };
}

#define BEGIN_DEF_OPTYPE(Name)													\
	struct Name { };                                                            \
	template <> const char Plan::OperationName<Name>::name[] = #Name;			\
	template <> struct Plan::OperationList<Name> {									\
		typedef type_list<
		
#define END_DEF_OPTYPE															\
		> type;																	\
	}

#define DEF_OPDETAILS(Name, Type)                         \
  template <> struct Plan::OperationDetailType<Name>      \
  { typedef Type type; };
  
#define DEF_OPDYNDURATION(Name, Num)                                               \
  template <> struct Plan::OperationDynamic<Name, Duration<Num> >                  \
  {                                                                                \
    enum { default = Num };                                                        \
    typedef OperationDetailType<OT>::type Details;                                 \
    static int getValueInternal(const Details& details);                           \
    static inline int getValue(void* d)                                            \
    { Details* details = (Details*) d; return getValueInternal(*d); }              \
  };                                                                               \
  template <> static int Plan::OperationDynamic<Name, Duration<Num> >::getValueInternal(const Details& details)

