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

template <class Traits>
class Operation
{
	public:
		typedef typename Traits::ResourceList	RLIST;
		typedef typename Traits::OperationList	OLIST;
		typedef Operation<Traits>				ThisType;
		typedef Resources<Traits>				ResourcesType;
		typedef OperationIndex<Traits>			IndexType;
		typedef ResourceIndex<Traits>			ResIndexType;

	public:
		explicit Operation(const IndexType& i, const TimeType& st = 0)
			: index_(i), status_(OperationStatus::scheduled), stage_(0), scheduledtime_(st), details_(NULL)
		{ }
		
		explicit Operation(const ThisType& o, const TimeType& st)
			: index_(o.index_), status_(o.status_), stage_(o.stage_), scheduledtime_(st), details_(o.details_)
		{ }
		
		TimeType duration() const
		{
			TimeType result = 0;
			idispatch<CountDuration, TimeType&>(result);
			return result - scheduledtime_;
		}

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
			idispatch<ChangeTimesInternal, std::set<TimeType>&>(result);
		}
		
		std::set<TimeType> changeTimes() const
		{
			std::set<TimeType> result;
			changeTimes(result);
			return result;
		}
		
		int stageCount() const
		{
			int result = 0;
			idispatch<StageCount, int&>(result);
			return result;
		}
		
		TimeType stageDuration(int stage) const
		{
			TimeType result = 0;
			idispatch<StageDuration, TimeType&, int&>(result, stage);
			return result;
		}
		
		bool isApplyable(const ResourcesType& res, int stage) const
		{
			ResIndexType blocking;
			return (firstApplyableAt(res, stage, blocking) == 0);
		}
		
		TimeType firstApplyableAt(const ResourcesType& res, int stage, ResIndexType& blocking) const
		{
			TimeType result = 0;
			idispatch<FirstApplyableAt, const ResourcesType&, TimeType&, int&, ResIndexType&>(res, result, stage, blocking);
			return result;
		}
		
		void apply(ResourcesType& res, const TimeInterval& interval) const
		{
			int counter = stage_;
 			idispatch<ApplyInternal, ResourcesType&, int&, const TimeInterval&>(res, counter, interval);
		}
		
		void execute(bool justactived)
		{
			if (justactived) {
				status_ = OperationStatus::started;
			}
			int counter = stage_;
			idispatch<ExecuteInternal, int&, OperationStatus::type&, TimeType&>(counter, status_, scheduledtime_);
			if (status_ == OperationStatus::completed) {
				++stage_;
				if (stage_ < stageCount())
					status_ = OperationStatus::started;
			}
		}
      
		std::string getName() const
		{
			return index_.getName();
		}
		  
		template <class OT>
		boost::shared_ptr< typename Plan::OperationDetailType<OT>::type> getDetails() const
		{
			return boost::static_pointer_cast<typename Plan::OperationDetailType<OT>::type>(details_);
		}
		 
		void setDetails(const DetailPointerType& ptr)
		{
			details_ = ptr;
		}
		
		OperationStatus::type status() const
		{
			return status_;
		}

	private:
		template <class OT, class T>
		struct CountDuration
		{
			static void call(const TimeType& time, const DetailPointerType& /*details*/, TimeType& result)
			{
				result = time;
			}
		};
		
	private:
		template <class OT, class T>
		struct ChangeTimesInternal
		{
			static void call(const TimeType& time, const DetailPointerType& /*details*/, std::set<TimeType>& timeSet)
			{
				timeSet.insert(time);
			}
		};
		
	private:
		template <class OT, class T>
		struct StageCount
		{
			static void call(const TimeType& /*time*/, const DetailPointerType& /*details*/, int& /*result*/)
			{ }
		};
	
		template <class OT, class CT, int num>
		struct StageCount< OT, CheckPoint<CT, num> >
		{
			static void call(const TimeType& /*time*/, const DetailPointerType& /*details*/, int& result)
			{
				result += 1;
			}
		};

	private:
		template <class OT, class T>
		struct StageDuration
		{
			static void call(const TimeType& /*time*/, const DetailPointerType& /*details*/, TimeType& /*result*/, int& /*stage*/)
			{ }
		};
	
		template <class OT, class CT, int num>
		struct StageDuration< OT, CheckPoint<CT, num> >
		{
			static void call(const TimeType& /*time*/, const DetailPointerType& /*details*/, TimeType& result, int& stage)
			{
				if (stage == 0)
					result += num;
				--stage;
			}
		};
		
	private:
		template <class OT, class T>
		struct FirstApplyableAt
		{
			static void call(const TimeType& /*time*/, const DetailPointerType& /*details*/, const ResourcesType& /*res*/, TimeType& /*result*/, int /*stage*/, ResIndexType& /*blocking*/)
			{ }
		};
		
		template <class OT, class CT, int num>
		struct FirstApplyableAt< OT, CheckPoint<CT, num> >
		{
			static void call(const TimeType& /*time*/, const DetailPointerType& /*details*/, const ResourcesType& /*res*/, TimeType& /*result*/, int stage, ResIndexType& /*blocking*/)
			{
				--stage;
			}
		};

		template <class OT, int num, class RT>
		struct FirstApplyableAt< OT, Needs<num, RT> >
		{
			static void call(const TimeType& /*time*/, const DetailPointerType& /*details*/, const ResourcesType& res, TimeType& result, int stage, ResIndexType& blocking)
			{
				if (stage == 0)
					if (res.template getExisting<RT>() < num) {
						result = std::numeric_limits<TimeType>::max();
						blocking = ResIndexType::template byClass<RT>();
					}
			}
		};
      
		template <class OT, int num, class RT>
		struct FirstApplyableAt< OT, Locks<num, RT> >
		{
			static void call(const TimeType& /*time*/, const DetailPointerType& /*details*/, const ResourcesType& res, TimeType& result, int stage, ResIndexType& blocking)
			{
				if (stage == 0)
					if (res.template get<RT>() < num) {
						result = std::numeric_limits<TimeType>::max();
						blocking = ResIndexType::template byClass<RT>();
					}
			}
		};
      
		template <class OT, int num, class RT>
		struct FirstApplyableAt< OT, Consums<num, RT> >
		{
			static void call(const TimeType& /*time*/, const DetailPointerType& /*details*/, const ResourcesType& res, TimeType& result, int stage, ResIndexType& blocking)
			{
				if (stage == 0)
					result = std::max(result, res.template firstMoreThan<RT>(num, blocking));
			}
		};

	private:
		template <class OT, class T>
		struct ApplyInternal
		{
			static void call(const TimeType& /*time*/, const DetailPointerType& /*details*/, ResourcesType& /*res*/, int& /*stage*/, const TimeInterval& /*interval*/)
			{ }
		};
		
		template <class OT, class CT, int num>
		struct ApplyInternal< OT, CheckPoint<CT, num> >
		{
			static void call(const TimeType& /*time*/, const DetailPointerType& /*details*/, ResourcesType& /*res*/, int& stage, const TimeInterval& /*interval*/)
			{
				--stage;
			}
		};
      
		template <class OT, int num, class RT>
		struct ApplyInternal< OT, Consums<num, RT> >
		{
			static void call(const TimeType& time, const DetailPointerType& /*details*/, ResourcesType& res, int& stage, const TimeInterval& interval)
			{
				if (stage <= 0)
					if ((interval.lower <= time) && (time <= interval.upper))
						res.template dec<RT>(time, num);
			}
		};

		template <class OT, int num, class RT>
		struct ApplyInternal< OT, Locks<num, RT> >
		{
			static void call(const TimeType& time, const DetailPointerType& /*details*/, ResourcesType& res, int& stage, const TimeInterval& interval)
			{
				if (stage <= 0)
					if ((interval.lower <= time) && (time <= interval.upper))
						res.template incLocked<RT>(time, num);
			}
		};
		
		template <class OT, int num, class RT>
		struct ApplyInternal< OT, Unlocks<num, RT> >
		{
			static void call(const TimeType& time, const DetailPointerType& /*details*/, ResourcesType& res, int& stage, const TimeInterval& interval)
			{
				if (stage <= 0)
					if ((interval.lower <= time) && (time <= interval.upper))
						res.template decLocked<RT>(time, num);
			}
		};

		template <class OT, int num, class RT>
		struct ApplyInternal< OT, Prods<num, RT> >
		{
			static void call(const TimeType& time, const DetailPointerType& /*details*/, ResourcesType& res, int& stage, const TimeInterval& interval)
			{
				if (stage <= 0)
					if ((interval.lower <= time) && (time <= interval.upper))
						res.template inc<RT>(time, num);
			}
		};
		
	private:
		template <class OT, class T>
		struct ExecuteInternal
		{
			static void call(const TimeType& /*time*/, const DetailPointerType& /*details*/, int& /*stage*/, OperationStatus::type& /*status*/, TimeType& /*scheduledtime*/)
			{ }
		};
		
		template <class OT, class CP, int num>
		struct ExecuteInternal< OT, CheckPoint<CP, num> >
		{
			static void call(const TimeType& /*time*/, const DetailPointerType& details, int& stage, OperationStatus::type& status, TimeType& scheduledtime)
			{
				if (stage == 0) {
					CheckPointResult::type res = Plan::CheckPointCode<CP, OT>::call(status, scheduledtime, details);
					switch (res)
					{
					case CheckPointResult::waiting:
						break;
						
					case CheckPointResult::running:
						status = OperationStatus::running;
						break;
						
					case CheckPointResult::completed:
						status = OperationStatus::completed;
						break;
					
					case CheckPointResult::failed:
						status = OperationStatus::failed;
						break;
					}
				}
				--stage;
			}
		};
		
	private:
		template <class OT, class T>
		struct docall
		{
			template <template<class, class> class DISPATCHER, class... ARGS>
			static void call(TimeType& /*time*/, const DetailPointerType& /*details*/, ARGS... /*args*/)
			{ }
		};
		
		template <class OT, class HT, int num>
		struct docall< OT, CheckPoint<HT, num> >
		{
			template <template<class, class> class DISPATCHER, class... ARGS>
			static void call(TimeType& time, const DetailPointerType& details, ARGS... /*args*/)
			{
				time += Plan::OperationDynamic< OT, CheckPoint<HT, num> >::getValue(details);
			}
		};
	
		template <class OT, class LIST>
		struct mydispatch2
		{
			template <template<class, class> class DISPATCHER, class... ARGS>
			static void call(TimeType& /*time*/, const DetailPointerType& /*details*/, ARGS... /*args*/)
			{ }
		};
		
		template <class OT, class FIRST, class ... TAIL>
		struct mydispatch2< OT, TL::type_list<FIRST, TAIL...> >
		{
			template <template<class, class> class DISPATCHER, class... ARGS>
			static void call(TimeType& time, const DetailPointerType& details, ARGS... args)
			{
				docall<OT, FIRST>::template call<DISPATCHER, ARGS...>(time, details, args...);
				DISPATCHER<OT, FIRST>::call(time, details, args...);
				mydispatch2< OT, TL::type_list<TAIL...> >::template call<DISPATCHER, ARGS...>(time, details, args...);
			}
		};
		
		template <class LIST>
		struct mydispatch
		{
			template <template<class, class> class DISPATCHER, class ... ARGS>
			static void call(int /*index*/, TimeType& /*time*/, const DetailPointerType& /*details*/, ARGS... /*args*/)
			{
				throw std::runtime_error("mydispatch<>::call() is called, but should not!");
			}
		};

		template <class FIRST, class ... TAIL>
		struct mydispatch< TL::type_list<FIRST, TAIL...> >
		{
			template <template<class, class> class DISPATCHER, class ... ARGS>
			static void call(int index, TimeType& time, const DetailPointerType& details, ARGS... args)
			{
				if (index == 0)
					mydispatch2< FIRST, typename Plan::OperationList<FIRST>::type >::template call<DISPATCHER, ARGS...>(time, details, args...);
				else 
					mydispatch< TL::type_list<TAIL...> >::template call<DISPATCHER, ARGS...>(index-1, time, details, args...);
			}
		};
		
		template <template <class, class> class DISPATCHER, class ... ARGS>
		void idispatch(ARGS... args) const
		{
			mydispatch<OLIST>::template call<DISPATCHER, ARGS...>(index_.getIndex(), scheduledtime_, details_, args...);
		}

	protected:
		IndexType				index_;
		OperationStatus::type 	status_;
		int						stage_;
		TimeType  				scheduledtime_;
		DetailPointerType	details_;
};
