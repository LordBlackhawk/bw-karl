#ifndef PLAN_h
#define PLAN_h

#include "resources.h"
#include "operations.h"

#include <vector>

struct FallbackBehaviourType
{
  enum type { Continue, Abort, Fail };
};

template <class RLIST, class OLIST>
class PlanContainer;

template <class RLIST, class OLIST>
struct DefaultFallbackBehaviour
{
	typedef Operation<RLIST,OLIST>			OperationType;
	typedef PlanContainer<RLIST, OLIST>		PlanType;
	FallbackBehaviourType::type operator () (PlanType& /*plan*/, const OperationType& /*op*/) const
	{
		std::cout << "!!!!Fallback\n";
		return FallbackBehaviourType::Continue;
	}
};

template <class RLIST, class OLIST>
class PlanContainer
{
	public:
		typedef Resources<RLIST>				ResourcesType;
		typedef Operation<RLIST,OLIST>			OperationType;
		typedef PlanContainer<RLIST, OLIST>		ThisType;
		
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
				
				Situation& operator ++ ()
				{
					std::set<int>::const_iterator it = parent.changetimes.lower_bound(currenttime+1);
					int newtime = (it == parent.changetimes.end()) ? parent.endtime+1 : *it;
					parent.evalOperations(current, currenttime+1, newtime);
					currenttime = newtime;
					return *this;
				}
				
				Situation& inc(int dt)
				{
					int newtime = currenttime + dt;
					parent.evalOperations(current, currenttime+1, newtime);
					currenttime = newtime;
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
				
				Situation(const ThisType& p, int time = -1) : parent(p), currenttime(time), current(parent.startres)
				{
					if (currenttime < 0)
						currenttime = parent.starttime;
					parent.evalOperations(current, parent.starttime, currenttime);
				}
		};

	public:
		PlanContainer(ResourcesType sr, int st=0)
			: startres(sr), starttime(st), opendtime(st), endtime(st)
		{ }
		
		void swap(ThisType& other)
		{
			startres.swap(other.startres);
			active_operations.swap(other.active_operations);
			scheduled_operations.swap(other.scheduled_operations);
			changetimes.swap(other.changetimes);
			std::swap(starttime, other.starttime);
			std::swap(endtime, other.endtime);
			std::swap(opendtime, other.opendtime);
		}
		
		friend void swap(ThisType& lhr, ThisType& rhs)
		{
			lhr.swap(rhs);
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
				bool applyable;
				while ( !(applyable = it.isApplyable(op, k)) && !it.beyond() )
					++it;
				
				if (!applyable)
					return false;
				
				it.inc(op.stageDuration(k));
			}
			add(op, it.time() - op.duration());
			return true;
		}
      
		template <class FallbackBehaviour>
		bool rebase(int timeinc, const ResourcesType& newres, const FallbackBehaviour& fbb)
		{
			ThisType newplan(newres, starttime+timeinc);
			
			for (auto it : active_operations) {
				it.execute(false);
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
						  swap(newplan);
						  return true;
						case FallbackBehaviourType::Continue:
						  break;
					}
				}
			}
			swap(newplan);
			return true;
		}
		
		bool rebase(int timeinc, const ResourcesType& newres)
		{
			return rebase(timeinc, newres, DefaultFallbackBehaviour<RLIST, OLIST>());
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
		
		OperationType& scheduled(int i)
		{
			return scheduled_operations[i];
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

#endif