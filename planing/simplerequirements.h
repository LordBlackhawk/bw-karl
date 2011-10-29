#pragma once

#include "typelists.h"

namespace Plan {
	template <class RT, class OT>
	struct SimpleRequirement
	{
		enum { value = false };
	};
}

#define DEF_SIMPLEREQUIREMENT(RName, OName)                            		\
    struct RName;															\
	struct OName;															\
	namespace Plan { template <> struct SimpleRequirement<RName, OName>  	\
	{ enum { value = true }; }; }

template <class OLIST, class RT>
struct SimpleRequirementList
{
	template <class OT>
	struct Predicate
	{ enum { value = Plan::SimpleRequirement<RT, OT>::value }; };

	typedef typename TL::sublist< Predicate, OLIST >::type type;
};
