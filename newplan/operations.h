#pragma once

#include "timetype.h"
#include "operationindex.h"
#include "resources.h"
#include "operationstatus.h"
#include "checkpoints.h"

#include <set>
#include <string>
#include <stdexcept>
#include <limits>

class Operation
{
	public:
		explicit Operation(const IndexType& i, const TimeType& st = 0)
			: index_(i), status_(OperationStatus::scheduled), stage_(0), scheduledtime_(st)
		{ }
		
		explicit Operation(const ThisType& o, const TimeType& st)
			: index_(o.index_), status_(o.status_), stage_(o.stage_), scheduledtime_(st), details_(o.details_)
		{ }
		
		// Auto-generated functions see bwplan.cpp
		TimeType duration() const;
		int stageCount() const;
		TimeType stageDuration(int stage) const;
		TimeType firstApplyableAt(const ResourcesType& res, int stage, ResIndexType& blocking) const;
		void apply(ResourcesType& res, const TimeInterval& interval, bool pushdecs = false) const;
		void execute(bool justactived);

		TimeType scheduledTime() const
		{
			return scheduledtime_;
		}

		TimeType scheduledEndtime() const
		{
			return scheduledtime_ + duration();
		}
		
		void changeTimes(std::set<TimeType>& result) const
		{
			TimeType curtime = time;
			result.insert(curtime);
			for (int k=0, size=stageCount(); k<size; ++k) {
				curtime += stageDuration(k);
				result.insert(curtime);
			}
		}
		
		std::set<TimeType> changeTimes() const
		{
			std::set<TimeType> result;
			changeTimes(result);
			return result;
		}
		
		bool isApplyable(const ResourcesType& res, int stage) const
		{
			ResIndexType blocking;
			return (firstApplyableAt(res, stage, blocking) == 0);
		}
      
		std::string getName() const
		{
			return index_.getName();
		}
		 
		void setDetails(const DetailPointerType& ptr)
		{
			details_ = ptr;
		}
		
		OperationStatus::type status() const
		{
			return status_;
		}

	protected:
		IndexType				index_;
		OperationStatus::type 	status_;
		int						stage_;
		TimeType  				scheduledtime_;
		DetailPointerType		details_;
};
