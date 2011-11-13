#pragma once

#include "resources.h"
#include "operations.h"
#include "checkpoints.h"

namespace {

template <class FUNC>
void Call(const FUNC& f, Operation& op)
{
	CheckPointResult::type res = f(op);
	switch (res)
	{
	case CheckPointResult::waiting:
		break;
		
	case CheckPointResult::running:
		op.status = OperationStatus::running;
		break;
		
	case CheckPointResult::completed:
		op.status = OperationStatus::completed;
		break;
	
	case CheckPointResult::failed:
		op.status = OperationStatus::failed;
		break;
	}
}

void Needs(const Resources& res, int num, const ResourceIndex& ri, TimeType& result, ResourceIndex& blocking)
{
	if (res.getExisting(ri) < num) {
		blocking = ri;
		result   = std::numeric_limits<TimeType>::max();
	}
}

void Locks(const Resources& res, int num, const ResourceIndex& ri, TimeType& result, ResourceIndex& blocking)
{
	if (res.get(ri) < num) {
		blocking = ri;
		result   = std::numeric_limits<TimeType>::max();
	}
}

void Consums(const Resources& res, int num, const ResourceIndex& ri, TimeType& result, ResourceIndex& blocking)
{
	if (res.get(ri) < num) {
		blocking = ri;
		switch (ri.getType())
		{
			case ResourceIndex::Minerals:
			case ResourceIndex::Gas:
			case ResourceIndex::Larva:
				int growth = res.getGrowth(ri);
				int value  = res.getInternalValue(ri);
				result = (num - value + growth-1)/growth;
				break;
			default:
				result = std::numeric_limits<TimeType>::max();
				break;
		}
	}
}

} // end namespace