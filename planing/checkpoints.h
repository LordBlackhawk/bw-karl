#pragma once

#include "optypes.h"
#include "operations.h"

struct CheckPointResult
{
	enum type { running, completed, failed };
};

namespace Plan {
	template <class CP>
	struct CheckPointName
	{ static const char name[]; };

	template <class CP, class OT>
	struct CheckPointCode
	{
		static CheckPointResult::type call(OperationStatus::type /*s*/, void* /*d*/)
		{ return CheckPointResult::completed; }
	};
}

#define DEF_CHECKPOINT(Name)																	\
	struct Name { };																			\
	template <> const char Plan::CheckPointName<Name>::name[] = #Name;

#define DEF_CHECKPOINTCODE(Name)																		\
	struct Name;																						\
	namespace Plan { template <class OT> struct CheckPointCode<Name, OT> {								\
		typedef OT OperationType;																		\
		typedef typename Plan::OperationDetailType<OT>::type Details;									\
		static CheckPointResult::type callInternal(OperationStatus::type status, Details& details);		\
		static CheckPointResult::type call(OperationStatus::type s, void* d)							\
		{ Details* details = (Details*) d; return call(s, *details); }									\
	}; }																								\
	template <class OT> CheckPointResult::type Plan::CheckPointCode<Name, OT>							\
	::callInternal(OperationStatus::type status, Details& details)

#define DEF_CHECKPOINTCODEEX(Name, OT)																		\
	struct Name;																							\
	struct OT;																								\
	namespace Plan { template <> struct CheckPointCode<Name, OT> {											\
		typedef OT OperationType;																			\
		typedef Plan::OperationDetailType<OT>::type Details;												\
		static CheckPointResult::type callInternal(OperationStatus::type status, Details& details);			\
		static CheckPointResult::type call(OperationStatus::type s, void* d)								\
		{ Details* details = (Details*) d; return call(s, *details); }										\
	}; }																									\
	template <> CheckPointResult::type Plan::CheckPointCode<Name, OT>										\
	::callInternal(OperationStatus::type status, Details& details)
