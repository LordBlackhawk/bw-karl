#pragma once

#include "typelists.h"

namespace Plan {
	template <class OT, class T>
	struct Associations
	{
		static T value;
	};
}

#define DEF_ASSOCIATION(Name, Type, Value)		 										\
	struct Name;																		\
	template <> static Type Plan::Associations<Name, Type>::value = Value;

template <class OT, class T>
T getAssociation()
{
	return Plan::Associations<OT, T>::value;
}
