#pragma once

#include "optypes.h"

namespace Plan {
	template <class OT, class T>
	struct OperationAssociations
	{
		static T value;
	};
   
  template <class RT, class OT>
  struct SimpleRequirement
  {
    enum { value = false };
  };
}

#define DEF_ASSOCIATION(Name, Type, Value)		 \
	struct Name;																\
	template <> static Type Plan::OperationAssociations<Name, Type>::value = Value;

template <class OT, class T>
T getOperationAssociation()
{
	return Plan::OperationAssociation<OT, T>::value;
}

#define DEF_SIMPLEREQUIREMENT(RName, OName)                            \
  namespace Plan { template <> struct SimpleRequirement<RName, OName>  \
  { enum { value = true }; }; }

template <class OLIST, class RT>
struct SimpleRequirementList
{
  template <class OT>
  struct Predicate
  { enum { value = Plan::SimpleRequirement<RT, OT>::value }; };
  
  typedef sublist< Predicate, OLIST >::type type;
};
