#pragma once

#include "operations.h"
#include "resourceindex.h"
#include "simplerequirements.h"

#include <set>
#include <iostream>

struct FallbackBehaviourType
{
  enum type { Continue, Abort, Fail, Success };
};

template <class Traits>
class PlanContainer;

template <class Traits>
struct DefaultFallbackBehaviour
{
	typedef Operation<Traits>			OperationType;
	typedef PlanContainer<Traits>		PlanType;
	typedef ResourceIndex<Traits>		ResIndexType;
	FallbackBehaviourType::type operator () (PlanType& /*plan*/, const OperationType& /*op*/, const ResIndexType& /*blocking*/) const
	{
		std::cout << "fallback!!!\n";
		return FallbackBehaviourType::Fail;
	}
};

template <class Traits, class NextBehaviour>
class SimpleFallbackBehaviour
{
	typedef typename Traits::ResourceList					RLIST;
	typedef typename Traits::OperationList					OLIST;
	typedef Operation<Traits>								OperationType;
	typedef PlanContainer<Traits>							PlanType;
	typedef ResourceIndex<Traits>							ResIndexType;
	typedef OperationIndex<Traits>							OperationIndexType;
	typedef SimpleFallbackBehaviour<Traits, NextBehaviour>	ThisType;
	
	public:
		SimpleFallbackBehaviour(NextBehaviour& nfbb) : level(0), nextfbb(nfbb)
		{ }
		
		FallbackBehaviourType::type operator () (PlanType& plan, const OperationType& op, const ResIndexType& blocking)
		{
			std::cout << "Operation " << op.getUserName() << " failed, problem resource is " << blocking.getUserName() << "\n";
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
			TL::dispatch<RLIST>::template call<TestOperation, PlanType&, ThisType&, FallbackBehaviourType::type&>
						(blocking.getIndex(), plan, *this, result);
			
			if (result == FallbackBehaviourType::Success)
				result = plan.push_back(op, *this);
				
			--level;
			if ((level == 0) && (result == FallbackBehaviourType::Fail)) {
				return nextfbb(plan, op, blocking);
			} else {
				return result;
			}
		}
		
	private:
		template <class OT>
		struct TestOperation2
		{
			static void call(PlanType& plan, ThisType& fbb, FallbackBehaviourType::type& result)
			{
				if (result != FallbackBehaviourType::Success) {
					OperationType newop = OperationType(OperationIndexType::template byClass<OT>());
					result = plan.push_back(newop, fbb);
				}
			}
		};
		
		template <class RT>
		struct TestOperation
		{
			static void call(PlanType& plan, ThisType& fbb, FallbackBehaviourType::type& result)
			{
				TL::enumerate<typename SimpleRequirementList<OLIST, RT>::type>
					::template call<TestOperation2, PlanType&, ThisType&, FallbackBehaviourType::type&>
						(plan, fbb, result);
			}
		};
		
	protected:
		std::set<ResIndexType>	tested;
		int 					level;
		NextBehaviour&			nextfbb;
};