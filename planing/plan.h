#ifndef PLAN_h
#define PLAN_h

#include "resources.h"
#include "operations.h"

#include <list>

struct FallbackBehaviourType
{
  enum type { continue, abort, fail };
};

FallbackBehaviourType::type DefaultFallbackBehaviour(Plan& plan, const Operation& op)
{
  return FallbackBehaviourType::fail;
}

template <class RLIST, class OLIST>
class Plan
{
	public:
		typedef Resources<RLIST>		ResourcesType;
		typedef Operation<RLIST,OLIST>	OperationType;
		typedef Plan<RLIST, OLIST>		ThisType;
		
	public:
		class Situation
		{
			friend class Plan;
			
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
					std::set<int>::const_iterator it = parent.changeTimes.lower_bound(currenttime+1);
					int newtime = (it == parent.changeTimes.end()) ? parent.endtime+1 : *it;
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
				
				bool isApplyable(const OperationType& op) const
				{
					return op.isApplyable(current);
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
			
			protected:
				const ThisType&	parent;
				int 	     	currenttime;
				ResourcesType	current;
				
				Situation(const ThisType& p, int time = p.starttime) : parent(p), currenttime(time), current(parent.startres)
				{
					parent.evalOperations(current, parent.starttime, time);
				}
		};

	public:
		Plan(ResourcesType sr, int st=0)
			: startres(sr), starttime(st), opendtime(0), endtime(0)
		{ }
		
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
			Situation it = opend(), itend = end();
			while ( (it != itend) && !it.isApplyable(op) )
				++it;
			
			if (it == end())
				return false;
				
			add(op, it.time());
			return true;
		}
      
    template <class FallbackBehaviour>
    bool rebase(int timeinc, const ResourceType& newres, const FallbackBehaviour& fbb = DefaultFallbackBehaviour)
    {
      ThisType newplan(newres, starttime+timeinc);
      for (auto it : operations) {
        if (!newplan.push_back(it)) {
          FallbackBehaviourType res = ffb(newplan, it);
          switch (res) {
            case FallbackBehaviourType::fail:
              return false;
            case FallbackBehaviourType::abort:
              std::swap(*this, newplan);
              return true;
            case FallbackBehaviourType::continue:
              break;
          }
        }
      }
      std::swap(*this, newplan);
      return true;
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
		
		int size() const
		{
			return operations.size();
		}

	protected:
		ResourcesType				startres;
		std::list<OperationType>	operations;
		std::set<int>				changeTimes;
    int              starttime;
		int							opendtime;
		int							endtime;

		void evalOperations(ResourcesType& res, int btime, int etime) const
		{
			for (auto it : operations)
				it.apply(res, btime, etime);
		}
		
		void add(const OperationType& op, int time)
		{
			OperationType newop(op, time);
			newop.changeTimes(changeTimes);
			operations.push_back(newop);
			opendtime = std::max(opendtime, time);
			endtime = *changeTimes.rbegin();
		}
};

#endif