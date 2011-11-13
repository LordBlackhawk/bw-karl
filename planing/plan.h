#pragma once

#include "timetype.h"
#include "resources.h"
#include "operations.h"
#include "fallbackbehaviour.h"
#include "linear-correction.h"
#include "utils/fileutils.h"

#include <boost/regex.hpp>
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
		typedef OperationIndex<Traits>			OpIndexType;
		typedef LinearCorrection<Traits>		CorrectionType;
		
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
				
				bool isApplyable(const OperationType& op, int stage) const
				{
					return op.isApplyable(current, stage);
				}
				
				TimeType firstApplyableAt(const OperationType& op, int stage, ResIndexType& blocking) const
				{
					TimeType value = currenttime + op.firstApplyableAt(current, stage, blocking);
					if (value < 0) value = std::numeric_limits<TimeType>::max();
					return value;
				}
				
				Situation& applyOperation(const OperationType& op, const TimeInterval& interval)
				{
					op.apply(current, interval);
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
				
				void update()
				{
					TimeType old = currenttime;
					current      = parent.startres;
					currenttime  = parent.starttime-1;
					advance(old);
				}
			
			protected:
				const ThisType&	parent;
				TimeType     	currenttime;
				ResourcesType	current;
				
				Situation(const ThisType& p, TimeType time = -1) : parent(p), currenttime(parent.starttime-1), current(parent.startres)
				{
					if (time < 0)
						time = parent.starttime;
					advance(time);
				}
            
				void advance(TimeType newtime)
				{
					current.advance(newtime-currenttime);
					TimeInterval interval(currenttime+1, newtime);
					parent.evalOperations(current, interval);
					currenttime = newtime;
				}
		};

	public:
		PlanContainer(ResourcesType sr, TimeType st=0)
			: startres(sr), starttime(st), opendtime(st), endtime(st)
		{
			startres.setTime(starttime);
			Traits::CorrectionTraits::addCorrections(*this, at(starttime));
		}
		
		bool empty() const
		{
			return scheduled_operations.empty() && active_operations.empty();
		}
		
		Situation at(TimeType time) const
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
				TimeType firstapplyable;
				ResIndexType blocking;
				while ((firstapplyable = it.firstApplyableAt(op, k, blocking)) > it.getNextTime())
					++it;
			
				if (firstapplyable == std::numeric_limits<TimeType>::max())
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
		bool rebase(TimeType timeinc, const ResourcesType& newres, FallbackBehaviour& fbb)
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
		
		bool rebase_sr(TimeType timeinc, const ResourcesType& newres)
		{
			DefaultFallbackBehaviour<Traits> dfbb;
			SimpleFallbackBehaviour< Traits, DefaultFallbackBehaviour<Traits> > sfbb(dfbb);
			return rebase(timeinc, newres, sfbb);
		}
		
		bool rebase_df(TimeType timeinc, const ResourcesType& newres)
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
		
		const std::vector<OperationType> scheduledOperations() const
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
		
		bool loadFromFile(const char* filename)
		{
			std::string content;
			if (!readFileToString(filename, content))
				return false;
			
			static boost::regex expression("^[[:space:]]*(\\*([[:digit:]]*))?[[:space:]]*([[:word:]]+)$");
			std::string::const_iterator start = content.begin(), end = content.end();
			boost::match_results<std::string::const_iterator> what;
			boost::match_flag_type flags = boost::match_default;
			while ((start != end) && regex_search(start, end, what, expression, flags))
			{
				start = what[0].second;
				// what[0] whole string.
				// what[2] Anzahl.
				// what[3] Operationsname.
				OpIndexType index = OpIndexType::byUserName(what.str(3));
				if (!index.valid())
					continue;
				int count = (what.str(2) != "") ? atoi(what.str(2).c_str()) : 1;
				for (int k=0; k<count; ++k)
					push_back_df(OperationType(index));
			}
			
			return true;
		}
		
		bool saveToFile(const char* filename) const
		{
			FILE* file = fopen(filename, "w");
			if (file == NULL)
				return false;
			for (auto it : scheduled_operations)
				fprintf(file, "%s\n", it.getUserName().c_str());
			fclose(file);
			return true;
		}
		
		void addCorrection(const CorrectionType& c)
		{
			corrections.push_back(c);
		}
		
		std::vector<CorrectionType> getCorrections() const
		{
			return corrections;
		}

	protected:
		ResourcesType				startres;
		std::vector<OperationType>	active_operations;
		std::vector<OperationType>	scheduled_operations;
		std::set<TimeType>			changetimes;
		std::vector<CorrectionType>	corrections;
		TimeType              		starttime;
		TimeType					opendtime;
		TimeType					endtime;

		void evalOperations(ResourcesType& res, const TimeInterval& interval) const
		{
			for (auto it : active_operations)
				it.apply(res, interval);
				
			for (auto it : scheduled_operations)
				it.apply(res, interval);
				
			for (auto it : corrections)
				it.apply(res, interval);
		}
		
		void add(const OperationType& op, TimeType time)
		{
			OperationType newop(op, time);
			newop.changeTimes(changetimes);
			scheduled_operations.push_back(newop);
			opendtime = std::max(opendtime, time);
			if (!changetimes.empty())
				endtime = *changetimes.rbegin();
			TimeType mintime = removeCorrections(time);
			Traits::CorrectionTraits::addCorrections(*this, at(mintime));
		}
		
		void addActive(const OperationType& op)
		{
			active_operations.push_back(op);
			op.changeTimes(changetimes);
			endtime = *changetimes.rbegin();
		}
		
		TimeType removeCorrections(const TimeType& time)
		{
			TimeType mintime = time;
			auto func = [time, &mintime] (const CorrectionType& c)
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
