#pragma once

#include "resources.h"
#include "operations.h"
#include "fallbackbehaviour.h"

#include <vector>

template <class Traits>
class PlanContainer
{
	public:
		typedef typename Traits::ResourceList	RLIST;
		typedef typename Traits::OperationList	OLIST;
		typedef Resources<Traits>				ResourcesType;
		typedef Operation<Traits>				OperationType;
		typedef PlanContainer<Traits>			ThisType;
		typedef ResourceIndex<Traits>			ResIndexType;
		
	public:
		class Situation
		{
			friend class PlanContainer;
			
			public:
				Situation& operator * ()
				{
					return *this;
				}
				
				const ResourcesType& operator -> () const
				{
					return current;
				}
				
				bool operator == (const Situation& arg) const
				{
					return (currenttime == arg.currenttime);
				}
				
				bool operator != (const Situation& arg) const
				{
					return !(*this == arg);
				}
				
				int getNextTime() const
				{
					std::set<int>::const_iterator it = parent.changetimes.lower_bound(currenttime+1);
					return (it == parent.changetimes.end()) ? std::numeric_limits<int>::max() : *it;
				}
				
				Situation& operator ++ ()
				{
					std::set<int>::const_iterator it = parent.changetimes.lower_bound(currenttime+1);
					int newtime = (it == parent.changetimes.end()) ? parent.endtime+1 : *it;
					advance(newtime);
					return *this;
				}
				
				Situation& inc(int dt)
				{
					advance(currenttime + dt);
 					return *this;
				}
				
				int time() const
				{
					return currenttime;
				}
				
				bool valid() const
				{
					return current.valid();
				}
				
				bool isApplyable(const OperationType& op, int stage) const
				{
					return op.isApplyable(current, stage);
				}
				
				int firstApplyableAt(const OperationType& op, int stage, ResIndexType& blocking) const
				{
					int value = currenttime + op.firstApplyableAt(current, stage, blocking);
					if (value < 0) value = std::numeric_limits<int>::max();
					return value;
				}
				
				Situation& applyOperation(const OperationType& op, int btime, int etime)
				{
					op.apply(current, btime, etime);
					return *this;
				}
				
				const ResourcesType& getResources() const
				{
					return current;
				}
				
				bool beyond() const
				{
					return (currenttime > parent.endtime);
				}
			
			protected:
				const ThisType&	parent;
				int 	     	currenttime;
				ResourcesType	current;
				
				Situation(const ThisType& p, int time = -1) : parent(p), currenttime(parent.starttime-1), current(parent.startres)
				{
					if (time < 0)
						time = parent.starttime;
					advance(time);
				}
            
				void advance(int newtime)
				{
					current.advance(newtime-currenttime);
					parent.evalOperations(current, currenttime+1, newtime);
					currenttime = newtime;
				}
		};

	public:
		PlanContainer(ResourcesType sr, int st=0)
			: startres(sr), starttime(st), opendtime(st), endtime(st)
		{
			startres.setTime(starttime);
		}
		
		bool empty() const
		{
			return scheduled_operations.empty() && active_operations.empty();
		}
		
		Situation at(int time) const
		{
			return Situation(*this, time);
		}
		
		Situation begin() const
		{
			return Situation(*this);
		}
		
		Situation opend() const
		{
			return at(opendtime);
		}
		
		Situation end() const
		{
			return at(endtime+1);
		}
		
		template <class FallbackBehaviour>
		FallbackBehaviourType::type push_back(const OperationType& op, FallbackBehaviour& fbb)
		{
			Situation it = opend();
			for (int k=0; k<op.stageCount(); ++k)
			{
				int firstapplyable;
				ResIndexType blocking;
				while ((firstapplyable = it.firstApplyableAt(op, k, blocking)) > it.getNextTime())
					++it;
			
				if (firstapplyable == std::numeric_limits<int>::max())
					return fbb(*this, op, blocking);
				
				it.inc(op.stageDuration(k) + firstapplyable - it.time());
			}
			add(op, it.time() - op.duration());
			return FallbackBehaviourType::Success;
		}
		
		FallbackBehaviourType::type push_back_sr(const OperationType& op)
		{
			DefaultFallbackBehaviour<Traits> dfbb;
			SimpleFallbackBehaviour< Traits, DefaultFallbackBehaviour<Traits> > sfbb(dfbb);
			return push_back(op, sfbb);
		}
		
		FallbackBehaviourType::type push_back_df(const OperationType& op)
		{
			DefaultFallbackBehaviour<Traits> dfbb;
			return push_back(op, dfbb);
		}
      
		template <class FallbackBehaviour>
		bool rebase(int timeinc, const ResourcesType& newres, FallbackBehaviour& fbb)
		{
			ThisType newplan(newres, starttime+timeinc);
			
			for (auto& it : active_operations) {
				it.execute(false);
				if (   (it.status() != OperationStatus::completed)
				    && (it.status() != OperationStatus::failed))
					newplan.addActive(it);
			}
			
			for (auto it : scheduled_operations) {
				FallbackBehaviourType::type res = newplan.push_back(it, fbb);
				switch (res)
				{
					case FallbackBehaviourType::Fail:
					  return false;
					case FallbackBehaviourType::Abort:
					  std::swap(*this, newplan);
					  return true;
					case FallbackBehaviourType::Continue:
					case FallbackBehaviourType::Success:
					  break;
				}
			}
			std::swap(*this, newplan);
			return true;
		}
		
		bool rebase_sr(int timeinc, const ResourcesType& newres)
		{
			DefaultFallbackBehaviour<Traits> dfbb;
			SimpleFallbackBehaviour< Traits, DefaultFallbackBehaviour<Traits> > sfbb(dfbb);
			return rebase(timeinc, newres, sfbb);
		}
		
		bool rebase_df(int timeinc, const ResourcesType& newres)
		{
			DefaultFallbackBehaviour<Traits> dfbb;
			return rebase(timeinc, newres, dfbb);
		}
		
		void execute()
		{
			while (!scheduled_operations.empty() && (scheduled_operations.front().scheduledTime() == starttime)) {
				active_operations.push_back(scheduled_operations.front());
				scheduled_operations.erase(scheduled_operations.begin());
				active_operations.back().execute(true);
			}
		}
		
		const ResourcesType& startResources() const
		{
			return startres;
		}
		
		int getStartTime() const
		{
			return starttime;
		}
    
		int endTime() const
		{
			return endtime;
		}
		
		int scheduledCount() const
		{
			return scheduled_operations.size();
		}
		
		const std::vector<OperationType> scheduledOperations() const
		{
			return scheduled_operations;
		}

	protected:
		ResourcesType				startres;
		std::vector<OperationType>	active_operations;
		std::vector<OperationType>	scheduled_operations;
		std::set<int>				changetimes;
		int              			starttime;
		int							opendtime;
		int							endtime;

		void evalOperations(ResourcesType& res, int btime, int etime) const
		{
			for (auto it : active_operations)
				it.apply(res, btime, etime);
				
			for (auto it : scheduled_operations)
				it.apply(res, btime, etime);
		}
		
		void add(const OperationType& op, int time)
		{
			OperationType newop(op, time);
			newop.changeTimes(changetimes);
			scheduled_operations.push_back(newop);
			opendtime = std::max(opendtime, time);
			if (!changetimes.empty())
				endtime = *changetimes.rbegin();
		}
		
		void addActive(const OperationType& op)
		{
			active_operations.push_back(op);
			op.changeTimes(changetimes);
			endtime = *changetimes.rbegin();
		}
};
