#pragma once

#include "operations.h"
#include "resourceindex.h"

#include <BWAPI.h>
#include <set>
#include <iostream>

class PlanContainer;

struct DefaultFallbackBehaviour
{
	FallbackBehaviourType::type operator () (PlanContainer& /*plan*/, const Operation& /*op*/, const ResourceIndex& /*blocking*/) const
	{
		std::clog << "fallback!!!\n";
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
			std::clog << BWAPI::Broodwar->getFrameCount() << ": Operation " << op.getName() << " failed, problem resource is " << blocking.getName() << "\n";
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