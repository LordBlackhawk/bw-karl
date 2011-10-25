#pragma once

#include "optypes.h"

namespace Plan {
	template <class OT, class T>
	struct OperationAssociations
	{
		static T value;
	};
}

#define DEF_ASSOCIATION(Name, Type, Value)																\
	namespace Operations { struct Name; }																\
	template <> static Type Plan::OperationAssociations<Operations::Name, Type>::value = Value;

template <class OT, class T>
T getOperationAssociation()
{
	return Plan::OperationAssociation<OT, T>::value;
}