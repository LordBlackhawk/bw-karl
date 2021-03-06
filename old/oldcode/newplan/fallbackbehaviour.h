#pragma once

#include "operations.h"
#include "resourceindex.h"

#include "utils/debug.h"

#include <BWAPI.h>
#include <set>

class PlanContainer;

struct DefaultFallbackBehaviour
{
	FallbackBehaviourType::type operator () (PlanContainer& /*plan*/, const Operation& /*op*/, const ResourceIndex& /*blocking*/) const
	{
		LOG2 << "fallback!!!";
		return FallbackBehaviourType::Fail;
	}
};

template <class NextBehaviour>
class SimpleFallbackBehaviour
{
	public:
		SimpleFallbackBehaviour(NextBehaviour& nfbb) : level(0), nextfbb(nfbb)
		{ }
		
		FallbackBehaviourType::type operator () (PlanContainer& plan, const Operation& op, const ResourceIndex& blocking)
		{
			LOG2 << "Operation " << op.getName() << " failed, problem resource is " << blocking.getName();
			if (!blocking.valid()) {
				if (level == 0)
					return nextfbb(plan, op, blocking);
				else
					return FallbackBehaviourType::Fail;
			}
			
			if (tested.find(blocking) != tested.end())
				return FallbackBehaviourType::Fail;			

			++level;
			tested.insert(blocking);
			
			FallbackBehaviourType::type result = FallbackBehaviourType::Fail;
			for (auto it : blocking.getAssociatedOperations())
			{
				result = plan.push_back(Operation(it), *this);
				if (result == FallbackBehaviourType::Success)
					break;
			}
			
			if (result == FallbackBehaviourType::Success)
				result = plan.push_back(op, *this);
				
			--level;
			if ((level == 0) && (result == FallbackBehaviourType::Fail)) {
				tested.clear();
				return nextfbb(plan, op, blocking);
			} else {
				return result;
			}
		}
		
	protected:
		std::set<ResourceIndex>	tested;
		int 					level;
		NextBehaviour&			nextfbb;
};