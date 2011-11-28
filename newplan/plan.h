#pragma once

#include "timetype.h"
#include "resources.h"
#include "operations.h"
#include "fallbackbehaviourtype.h"
#include "linear-correction.h"

#include "utils/pair-container.h"
#include "utils/debug.h"

#include <vector>

class PlanContainer
{
	typedef PlanContainer ThisType;
	public:
		class Situation
		{
			friend class PlanContainer;
			
			public:
				Situation& operator * ()
				{
					return *this;
				}
				
				const Resources& operator -> () const
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
				
				TimeType getNextTime() const
				{
					std::set<TimeType>::const_iterator it = parent.changetimes.lower_bound(currenttime+1);
					return (it == parent.changetimes.end()) ? std::numeric_limits<TimeType>::max() : *it;
				}
				
				Situation& operator ++ ()
				{
					std::set<TimeType>::const_iterator it = parent.changetimes.lower_bound(currenttime+1);
					TimeType newtime = (it == parent.changetimes.end()) ? parent.endtime+1 : *it;
					advance(newtime);
					return *this;
				}
				
				Situation& inc(TimeType dt)
				{
					advance(currenttime + dt);
 					return *this;
				}
				
				TimeType time() const
				{
					return currenttime;
				}
				
				bool valid() const
				{
					return current.valid();
				}
				
				bool isApplyable(const Operation& op, int stage) const
				{
					return op.isApplyable(current, stage);
				}
				
				TimeType firstApplyableAt(const Operation& op, int stage, ResourceIndex& blocking) const
				{
					TimeType value = currenttime + op.firstApplyableAt(current, stage, blocking);
					if (value < 0) value = std::numeric_limits<TimeType>::max();
					return value;
				}
				
				Situation& applyOperation(const Operation& op, const TimeInterval& interval)
				{
					op.apply(current, interval, pushdecs);
					return *this;
				}
				
				const Resources& getResources() const
				{
					return current;
				}
				
				bool beyond() const
				{
					return (currenttime > parent.endtime);
				}
				
				void update()
				{
					TimeType old = currenttime;
					current      = parent.startres;
					currenttime  = parent.starttime;
					init();
					advance(old);
				}
			
			protected:
				const ThisType&	parent;
				TimeType     	currenttime;
				Resources		current;
				bool 			pushdecs;
				
				Situation(const ThisType& p, TimeType time = -1, bool pd = false)
					: parent(p), currenttime(parent.starttime), current(parent.startres), pushdecs(pd)
				{
					if (time < 0)
						time = parent.starttime;
					init();
					advance(time);
				}
				
				void init()
				{
					if (pushdecs)
					{
						TimeInterval interval(currenttime, currenttime);
						parent.evalOperations(current, interval, pushdecs);
					}
				}
            
				void advance(TimeType newtime)
				{
					if (newtime == currenttime)
						return;
					current.advance(newtime-currenttime);
					TimeInterval interval(currenttime, newtime);
					parent.evalOperations(current, interval, pushdecs);
					currenttime = newtime;
				}
		};

	public:
		PlanContainer(Resources sr, TimeType st=0)
			: startres(sr), starttime(st), opendtime(st), endtime(st)
		{
			startres.setTime(starttime);
			addCorrections(starttime);
		}
		
		PlanContainer(const Resources& sr, TimeType st, const std::vector<Operation>& actives)
			: startres(sr), active_operations(actives), starttime(st), opendtime(st), endtime(st)
		{
			startres.setTime(starttime);
			addCorrections(starttime);
		}
		
		bool empty() const
		{
			return scheduled_operations.empty() && active_operations.empty();
		}
		
		Situation at(TimeType time, bool pushdecs = false) const
		{
			return Situation(*this, time, pushdecs);
		}
		
		Situation begin(bool pushdecs = false) const
		{
			return Situation(*this, -1, pushdecs);
		}
		
		Situation opend(bool pushdecs = false) const
		{
			return at(opendtime, pushdecs);
		}
		
		Situation end() const
		{
			return at(endtime+1);
		}
		
		template <class FallbackBehaviour>
		FallbackBehaviourType::type push_back(const Operation& op_, FallbackBehaviour& fbb)
		{
			Operation op = op_;
			Situation it = opend(true);
			TimeType current_duration = 0;
			for (int k=0; k<op.stageCount(); ++k)
			{
				TimeType firstapplyable;
				ResourceIndex blocking;
				while ((firstapplyable = it.firstApplyableAt(op, k, blocking)) > it.getNextTime())
					++it;
			
				if (firstapplyable == std::numeric_limits<TimeType>::max())
					return fbb(*this, op, blocking);
				
				op.rescheduleBegin(firstapplyable - current_duration);
				current_duration  += op.stageDuration(k);
				TimeInterval interval(-1, firstapplyable + op.stageDuration(k)-1);
				it.update();
				it.applyOperation(op, interval);
				
				it.inc(firstapplyable + op.stageDuration(k) - it.time());
			}
			add(op, it.time() - op.duration());
			return FallbackBehaviourType::Success;
		}
		
		FallbackBehaviourType::type push_back_sr(const Operation& op);
		FallbackBehaviourType::type push_back_df(const Operation& op);
      
		template <class FallbackBehaviour>
		bool rebase(TimeType timeinc, const Resources& newres, FallbackBehaviour& fbb)
		{
			ThisType newplan(newres, starttime+timeinc);
			
			for (auto& it : active_operations) {
				if (it.status() == OperationStatus::failed) {
					LOG1 << "Operation " << it.getName() << " failed, before rebase.";
					continue;
				} else if (it.status() == OperationStatus::completed) {
					continue;
				}
				it.execute(false);
				if (it.status() == OperationStatus::failed) {
					LOG1 << "Operation " << it.getName() << " failed, while rebase.";
				} else if (it.status() != OperationStatus::completed) {
					newplan.addActive(it);
				}
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
		
		bool rebase_sr(TimeType timeinc, const Resources& newres);
		bool rebase_df(TimeType timeinc, const Resources& newres);
		
		template <class Function>
		bool optimizeLocal(const Function& f)
		{
			PlanContainer newplan(startres, starttime, active_operations);
			bool skipnext = false;
			bool changed  = false;

			for (auto it : pairsOf(scheduled_operations))
			{
				if (skipnext) {
					skipnext = false;
					continue;
				}

				PlanContainer keep   = newplan;
				PlanContainer change = newplan;

				Operation first = *it.first, second = *it.second;

				if (!keep.push_back_df(first) || !keep.push_back_df(second)) {
					LOG1 << "Something went wrong. Original order is not possible any more.";
					return false;
				}

				if (!change.push_back_df(first) || !change.push_back_df(second) || f(keep, change)) {
					newplan.push_back_df(first);
					continue;
				}

				newplan  = change;
				skipnext = true;
				changed  = true;
			}

			if (!skipnext)
				newplan.push_back_df(scheduled_operations.back());

			if (f(*this, newplan))
				LOG1 << "Elementar Steps are better, but overall plan is not better! Nevertheless taking the new.";

			std::swap(*this, newplan);
			return changed;
		}

		template <class Function>
		int optimizeLocalMulti(const Function& f, int max_counter = 100)
		{
			int counter = 1;
			while (optimizeLocal(f) && (counter < max_counter))
				++counter;
			return counter;
		}

		int optimizeEndTime(int max_counter = 100);
		
		void execute()
		{
			while (!scheduled_operations.empty() && (scheduled_operations.front().scheduledTime() == starttime)) {
				active_operations.push_back(scheduled_operations.front());
				scheduled_operations.erase(scheduled_operations.begin());
				active_operations.back().execute(true);
			}
		}
		
		const Resources& startResources() const
		{
			return startres;
		}
		
		TimeType getStartTime() const
		{
			return starttime;
		}
    
		TimeType endTime() const
		{
			return endtime;
		}
		
		int scheduledCount() const
		{
			return scheduled_operations.size();
		}
		
		const std::vector<Operation> scheduledOperations() const
		{
			return scheduled_operations;
		}
		
		void clear()
		{
			active_operations.clear();
			scheduled_operations.clear();
			changetimes.clear();
			opendtime = endtime = starttime;
		}
		
		bool loadFromFile(const char* filename);
		bool saveToFile(const char* filename) const;
		
		void addCorrection(const LinearCorrection& c)
		{
			corrections.push_back(c);
		}
		
		std::vector<LinearCorrection> getCorrections() const
		{
			return corrections;
		}
		
		void addtime(TimeType t)
		{
			changetimes.insert(t);
		}

	protected:
		Resources						startres;
		std::vector<Operation>			active_operations;
		std::vector<Operation>			scheduled_operations;
		std::set<TimeType>				changetimes;
		std::vector<LinearCorrection>	corrections;
		TimeType              			starttime;
		TimeType						opendtime;
		TimeType						endtime;

		void evalOperations(Resources& res, const TimeInterval& interval, bool pushdecs) const
		{
			for (auto it : active_operations)
				it.apply(res, interval, pushdecs);
				
			for (auto it : scheduled_operations)
				it.apply(res, interval, pushdecs);
				
			for (auto it : corrections)
				it.apply(res, interval);
		}
		
		// see bwplan.cpp & add-linear-corection.h.
		void addCorrections(const TimeType& starttime);
		
		void add(const Operation& op, TimeType time)
		{
			Operation newop(op, time);
			newop.changeTimes(changetimes);
			scheduled_operations.push_back(newop);
			opendtime = std::max(opendtime, time);
			if (!changetimes.empty())
				endtime = *changetimes.rbegin();
			TimeType mintime = removeCorrections(time);
			addCorrections(mintime);
		}
		
		void addActive(const Operation& op)
		{
			active_operations.push_back(op);
			Operation& newop = active_operations.back();
			if (newop.scheduledTime() <= starttime)
				newop.rescheduleBegin(starttime + 1);
			newop.changeTimes(changetimes);
			endtime = *changetimes.rbegin();
		}
		
		TimeType removeCorrections(const TimeType& time)
		{
			TimeType mintime = time;
			auto func = [time, &mintime] (const LinearCorrection& c)
				{ 
					bool res = c.isLaterAs(time);
					if (res)
						mintime = std::min(mintime, c.interval.lower);
					return res;
				};
			auto end = std::remove_if(corrections.begin(), corrections.end(), func);
			corrections.resize(end-corrections.begin());
			return mintime;
		}
};
