#pragma once

#include "timetype.h"
#include "optypes.h"
#include "operationstatus.h"

#include <boost/shared_ptr.hpp>

struct CheckPointResult
{
	enum type { waiting, running, completed, failed };
};

namespace Plan {
	template <class CP>
	struct CheckPointName
	{ static const char name[]; };

	template <class CP, class OT>
	struct CheckPointCode
	{
		static CheckPointResult::type call(OperationStatus::type /*s*/, int& /*scheduledtime*/, int* /*d*/)
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
		typedef boost::shared_ptr<typename Plan::OperationDetailType<OT>::type> Details;									\
		const OperationStatus::type status;																\
		TimeType& scheduledtime;																				\
		Details& details;																			\
		CheckPointCode(OperationStatus::type s, TimeType& st, Details& d)									\
			: status(s), scheduledtime(st), details(d) { }												\
		CheckPointResult::type callInternal();															\
		static CheckPointResult::type call(OperationStatus::type s, TimeType& st, Details& d)					\
		{ return CheckPointCode<Name, OT>(s, st, d).callInternal(); }						\
	}; }																								\
	template <class OT> CheckPointResult::type Plan::CheckPointCode<Name, OT>::callInternal()

#define DEF_CHECKPOINTCODEEX(Name, OT)																	\
	struct Name;																						\
	struct OT;																							\
	namespace Plan { template <> struct CheckPointCode<Name, OT> {										\
		typedef OT OperationType;																		\
		typedef boost::shared_ptr<typename Plan::OperationDetailType<OT>::type> Details;											\
		const OperationStatus::type status;																\
		TimeType& scheduledtime;																				\
		Details& details;																			\
		CheckPointCode(OperationStatus::type s, TimeType& st, Details& d)									\
			: status(s), scheduledtime(st), details(d) { }												\
		CheckPointResult::type callInternal();															\
		static CheckPointResult::type call(OperationStatus::type s, TimeType& st, Details& d)					\
		{ return CheckPointCode<Name, OT>(s, st, d).callInternal(); }						\
	}; }																								\
	CheckPointResult::type Plan::CheckPointCode<Name, OT>::callInternal()
