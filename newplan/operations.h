#pragma once

#include "timetype.h"
#include "operationindex.h"
#include "resources.h"
#include "operationstatus.h"
#include "checkpoints.h"

#include <boost/shared_ptr.hpp>

#include <set>
#include <string>
#include <stdexcept>
#include <limits>

typedef boost::shared_ptr<void> DetailPointerType;

class Operation
{
	typedef Operation ThisType;
	
	public:
		explicit Operation(const OperationIndex& i, const TimeType& st = 0)
			: index_(i), status_(OperationStatus::scheduled), stage_(0), scheduledtime_(st)
		{ }
		
		explicit Operation(const ThisType& o, const TimeType& st)
			: index_(o.index_), status_(o.status_), stage_(o.stage_), scheduledtime_(st), details_(o.details_)
		{ }
		
		// Auto-generated functions see bwplan.cpp
		TimeType duration() const;
		int stageCount() const;
		TimeType stageDuration(int stage) const;
		TimeType firstApplyableAt(const Resources& res, int stage, ResourceIndex& blocking) const;
		void apply(Resources& res, const TimeInterval& interval, bool pushdecs = false) const;
		void execute(bool justactived);

		TimeType scheduledTime() const
		{
			return scheduledtime_;
		}
		
		TimeType& scheduledTime()
		{
			return scheduledtime_;
		}

		TimeType scheduledEndtime() const
		{
			return scheduledtime_ + duration();
		}
		
		void changeTimes(std::set<TimeType>& result) const
		{
			TimeType curtime = scheduledtime_;
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
		
		bool isApplyable(const Resources& res, int stage) const
		{
			ResourceIndex blocking;
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
		
		template <class T>
		boost::shared_ptr<T> getDetails()
		{
			if (details_.use_count() == 0)
				details_ = boost::shared_ptr<T>(new T());
			return boost::static_pointer_cast<T>(details_);
		}
		
		OperationStatus::type status() const
		{
			return status_;
		}
		
		OperationStatus::type& status()
		{
			return status_;
		}
	
	#ifndef NO_ASSOCIATIONS
		BWAPI::Race associatedRace() const
		{
			return index_.associatedRace();
		}
		
		BWAPI::UnitType associatedUnitType() const
		{
			return index_.associatedUnitType();
		}
	
		BWAPI::TechType associatedTechType() const
		{
			return index_.associatedTechType();
		}
		
		BWAPI::UpgradeType associatedUpgradeType() const
		{
			return index_.associatedUpgradeType();
		}
	#endif

	protected:
		OperationIndex			index_;
		OperationStatus::type 	status_;
		int						stage_;
		TimeType  				scheduledtime_;
		DetailPointerType		details_;
};
