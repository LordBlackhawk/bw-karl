#pragma once

#include "optypes.h"

struct OperationStatus
{
	enum type { scheduled, started, running, completed, failed };
};

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
		typedef typename Plan::OperationDetailType<OT>::type Details;									\
		const OperationStatus::type status;																\
		int& scheduledtime;																				\
		const Details* details;																			\
		CheckPointCode(OperationStatus::type s, int& st, Details* d)									\
			: status(s), scheduledtime(st), details(d) { }												\
		CheckPointResult::type callInternal();															\
		static CheckPointResult::type call(OperationStatus::type s, int& st, int* d)					\
		{ return CheckPointCode<Name, OT>(s, st, (Details*) d).callInternal(); }						\
	}; }																								\
	template <class OT> CheckPointResult::type Plan::CheckPointCode<Name, OT>::callInternal()

#define DEF_CHECKPOINTCODEEX(Name, OT)																	\
	struct Name;																						\
	struct OT;																							\
	namespace Plan { template <> struct CheckPointCode<Name, OT> {										\
		typedef OT OperationType;																		\
		typedef Plan::OperationDetailType<OT>::type Details;											\
		const OperationStatus::type status;																\
		int& scheduledtime;																				\
		const Details* details;																			\
		CheckPointCode(OperationStatus::type s, int& st, Details* d)									\
			: status(s), scheduledtime(st), details(d) { }												\
		CheckPointResult::type callInternal();															\
		static CheckPointResult::type call(OperationStatus::type s, int& st, int* d)					\
		{ return CheckPointCode<Name, OT>(s, st, (Details*) d).callInternal(); }						\
	}; }																								\
	CheckPointResult::type Plan::CheckPointCode<Name, OT>::callInternal()
