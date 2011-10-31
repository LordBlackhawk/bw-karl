#pragma once

#include "resources.h"
#include "operations.h"

#include <vector>

struct FallbackBehaviourType
{
  enum type { Continue, Abort, Fail };
};

template <class Traits>
class PlanContainer;

template <class Traits>
struct DefaultFallbackBehaviour
{
	typedef Operation<Traits>			OperationType;
	typedef PlanContainer<Traits>		PlanType;
	FallbackBehaviourType::type operator () (PlanType& /*plan*/, const OperationType& /*op*/) const
	{
		//std::cout << "!!!!Fallback\n";
		return FallbackBehaviourType::Continue;
	}
};

template <class Traits>
class PlanContainer
{
	public:
		typedef typename Traits::ResourceList	RLIST;
		typedef typename Traits::OperationList	OLIST;
		typedef Resources<Traits>				ResourcesType;
		typedef Operation<Traits>				OperationType;
		typedef PlanContainer<Traits>			ThisType;
		
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
				
				int firstApplyableAt(const OperationType& op, int stage) const
				{
					int value = currenttime + op.firstApplyableAt(current, stage);
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
		
		bool push_back(const OperationType& op)
		{
			Situation it = opend();
			for (int k=0; k<op.stageCount(); ++k)
			{
				int firstapplyable;
				while ((firstapplyable = it.firstApplyableAt(op, k)) > it.getNextTime())
					++it;
			
				if (firstapplyable == std::numeric_limits<int>::max())
					return false;
				
				it.inc(op.stageDuration(k) + firstapplyable - it.time());
			}
			add(op, it.time() - op.duration());
			return true;
		}
      
		template <class FallbackBehaviour>
		bool rebase(int timeinc, const ResourcesType& newres, const FallbackBehaviour& fbb)
		{
			ThisType newplan(newres, starttime+timeinc);
			
			for (auto& it : active_operations) {
				it.execute(false);
				if (   (it.status() != OperationStatus::completed)
				    && (it.status() != OperationStatus::failed))
					newplan.addActive(it);
			}
			
			for (auto it : scheduled_operations) {
				if (!newplan.push_back(it)) {
					FallbackBehaviourType::type res = fbb(newplan, it);
					switch (res)
					{
						case FallbackBehaviourType::Fail:
						  return false;
						case FallbackBehaviourType::Abort:
						  std::swap(*this, newplan);
						  return true;
						case FallbackBehaviourType::Continue:
						  break;
					}
				}
			}
			std::swap(*this, newplan);
			return true;
		}
		
		bool rebase(int timeinc, const ResourcesType& newres)
		{
			return rebase(timeinc, newres, DefaultFallbackBehaviour<Traits>());
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
			endtime = *changetimes.rbegin();
		}
		
		void addActive(const OperationType& op)
		{
			active_operations.push_back(op);
			op.changeTimes(changetimes);
			endtime = *changetimes.rbegin();
		}
};
