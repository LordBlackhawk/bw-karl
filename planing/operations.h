#pragma once

#include "operationindex.h"
#include "resources.h"
#include "operationstatus.h"
#include "checkpoints.h"

#include <set>
#include <string>
#include <stdexcept>
#include <limits>

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
		explicit Operation(const IndexType& i, int st = 0)
			: index_(i), status_(OperationStatus::scheduled), stage_(0), scheduledtime_(st), details_(NULL)
		{ }
		
		explicit Operation(const ThisType& o, int st)
			: index_(o.index_), status_(o.status_), stage_(o.stage_), scheduledtime_(st), details_(o.details_)
		{ }
		
		int duration() const
		{
			int result = 0;
			idispatch<CountDuration, int&>(result);
			return result - scheduledtime_;
		}

		int scheduledTime() const
		{
			return scheduledtime_;
		}

		int scheduledEndtime() const
		{
			return scheduledtime_ + duration();
		}
		
		void changeTimes(std::set<int>& result) const
		{
			idispatch<ChangeTimesInternal, std::set<int>&>(result);
		}
		
		std::set<int> changeTimes() const
		{
			std::set<int> result;
			changeTimes(result);
			return result;
		}
		
		int stageCount() const
		{
			int result = 0;
			idispatch<StageCount, int&>(result);
			return result;
		}
		
		int stageDuration(int stage) const
		{
			int result = 0;
			idispatch<StageDuration, int&, int&>(result, stage);
			return result;
		}
		
		bool isApplyable(const ResourcesType& res, int stage) const
		{
			ResIndexType blocking;
			return (firstApplyableAt(res, stage, blocking) == 0);
		}
		
		int firstApplyableAt(const ResourcesType& res, int stage, ResIndexType& blocking) const
		{
			int result = 0;
			idispatch<FirstApplyableAt, const ResourcesType&, int&, int&, ResIndexType&>(res, result, stage, blocking);
			return result;
		}
		
		void apply(ResourcesType& res, int btime, int etime) const
		{
			int counter = stage_;
 			idispatch<ApplyInternal, ResourcesType&, int&, int, int>(res, counter, btime, etime);
		}
		
		void execute(bool justactived)
		{
			if (justactived) {
				status_ = OperationStatus::started;
			}
			int counter = stage_;
			idispatch<ExecuteInternal, int&, OperationStatus::type&, int&>(counter, status_, scheduledtime_);
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
		
		std::string getUserName() const
		{
			return index_.getUserName();
		}
		  
		template <class OT>
		typename Plan::OperationDetailType<OT>::type* getDetails()
		{
			return (typename Plan::OperationDetailType<OT>::type*) details_;
		}
		 
		void setDetails(void* ptr)
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
			static void call(int time, int* /*details*/, int& result)
			{
				result = time;
			}
		};
		
	private:
		template <class OT, class T>
		struct ChangeTimesInternal
		{
			static void call(int time, int* /*details*/, std::set<int>& timeSet)
			{
				timeSet.insert(time);
			}
		};
		
	private:
		template <class OT, class T>
		struct StageCount
		{
			static void call(int /*time*/, int* /*details*/, int& /*result*/)
			{ }
		};
	
		template <class OT, class CT, int num>
		struct StageCount< OT, CheckPoint<CT, num> >
		{
			static void call(int /*time*/, int* /*details*/, int& result)
			{
				result += 1;
			}
		};

	private:
		template <class OT, class T>
		struct StageDuration
		{
			static void call(int /*time*/, int* /*details*/, int& /*result*/, int& /*stage*/)
			{ }
		};
	
		template <class OT, class CT, int num>
		struct StageDuration< OT, CheckPoint<CT, num> >
		{
			static void call(int /*time*/, int* /*details*/, int& result, int& stage)
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
			static void call(int /*time*/, int* /*details*/, const ResourcesType& /*res*/, int& /*result*/, int /*stage*/, ResIndexType& /*blocking*/)
			{ }
		};
		
		template <class OT, class CT, int num>
		struct FirstApplyableAt< OT, CheckPoint<CT, num> >
		{
			static void call(int /*time*/, int* /*details*/, const ResourcesType& /*res*/, int& /*result*/, int stage, ResIndexType& /*blocking*/)
			{
				--stage;
			}
		};

		template <class OT, int num, class RT>
		struct FirstApplyableAt< OT, Needs<num, RT> >
		{
			static void call(int /*time*/, int* /*details*/, const ResourcesType& res, int& result, int stage, ResIndexType& blocking)
			{
				if (stage == 0)
					if (res.template getExisting<RT>() < num) {
						result = std::numeric_limits<int>::max();
						blocking = ResIndexType::template byClass<RT>();
					}
			}
		};
      
		template <class OT, int num, class RT>
		struct FirstApplyableAt< OT, Locks<num, RT> >
		{
			static void call(int /*time*/, int* /*details*/, const ResourcesType& res, int& result, int stage, ResIndexType& blocking)
			{
				if (stage == 0)
					if (res.template get<RT>() < num) {
						result = std::numeric_limits<int>::max();
						blocking = ResIndexType::template byClass<RT>();
					}
			}
		};
      
		template <class OT, int num, class RT>
		struct FirstApplyableAt< OT, Consums<num, RT> >
		{
			static void call(int /*time*/, int* /*details*/, const ResourcesType& res, int& result, int stage, ResIndexType& blocking)
			{
				if (stage == 0)
					result = std::max(result, res.template firstMoreThan<RT>(num, blocking));
			}
		};

	private:
		template <class OT, class T>
		struct ApplyInternal
		{
			static void call(int /*time*/, int* /*details*/, ResourcesType& /*res*/, int& /*stage*/, int /*btime*/, int /*etime*/)
			{ }
		};
		
		template <class OT, class CT, int num>
		struct ApplyInternal< OT, CheckPoint<CT, num> >
		{
			static void call(int /*time*/, int* /*details*/, ResourcesType& /*res*/, int& stage, int /*btime*/, int /*etime*/)
			{
				--stage;
			}
		};
      
		template <class OT, int num, class RT>
		struct ApplyInternal< OT, Consums<num, RT> >
		{
			static void call(int time, int* /*details*/, ResourcesType& res, int& stage, int btime, int etime)
			{
				if (stage <= 0)
					if ((btime <= time) && (time <= etime))
						res.template dec<RT>(time, num);
			}
		};

		template <class OT, int num, class RT>
		struct ApplyInternal< OT, Locks<num, RT> >
		{
			static void call(int time, int* /*details*/, ResourcesType& res, int& stage, int btime, int etime)
			{
				if (stage <= 0)
					if ((btime <= time) && (time <= etime))
						res.template incLocked<RT>(time, num);
			}
		};
		
		template <class OT, int num, class RT>
		struct ApplyInternal< OT, Unlocks<num, RT> >
		{
			static void call(int time, int* /*details*/, ResourcesType& res, int& stage, int btime, int etime)
			{
				if (stage <= 0)
					if ((btime <= time) && (time <= etime))
						res.template decLocked<RT>(time, num);
			}
		};

		template <class OT, int num, class RT>
		struct ApplyInternal< OT, Prods<num, RT> >
		{
			static void call(int time, int* /*details*/, ResourcesType& res, int& stage, int btime, int etime)
			{
				if (stage <= 0)
					if ((btime <= time) && (time <= etime))
						res.template inc<RT>(time, num);
			}
		};
		
	private:
		template <class OT, class T>
		struct ExecuteInternal
		{
			static void call(int /*time*/, int* /*details*/, int& /*stage*/, OperationStatus::type& /*status*/, int& /*scheduledtime*/)
			{ }
		};
		
		template <class OT, class CP, int num>
		struct ExecuteInternal< OT, CheckPoint<CP, num> >
		{
			static void call(int /*time*/, int* details, int& stage, OperationStatus::type& status, int& scheduledtime)
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
			static void call(int /*time*/, int* /*details*/, ARGS... /*args*/)
			{ }
		};
		
		template <class OT, class HT, int num>
		struct docall< OT, CheckPoint<HT, num> >
		{
			template <template<class, class> class DISPATCHER, class... ARGS>
			static void call(int& time, int* details, ARGS... /*args*/)
			{
				time += Plan::OperationDynamic< OT, CheckPoint<HT, num> >::getValue(details);
			}
		};
	
		template <class OT, class LIST>
		struct mydispatch2
		{
			template <template<class, class> class DISPATCHER, class... ARGS>
			static void call(int /*time*/, int* /*details*/, ARGS... /*args*/)
			{ }
		};
		
		template <class OT, class FIRST, class ... TAIL>
		struct mydispatch2< OT, TL::type_list<FIRST, TAIL...> >
		{
			template <template<class, class> class DISPATCHER, class... ARGS>
			static void call(int time, int* details, ARGS... args)
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
			static void call(int /*index*/, int /*time*/, int* /*details*/, ARGS... /*args*/)
			{
				throw std::runtime_error("mydispatch<>::call() is called, but should not!");
			}
		};

		template <class FIRST, class ... TAIL>
		struct mydispatch< TL::type_list<FIRST, TAIL...> >
		{
			template <template<class, class> class DISPATCHER, class ... ARGS>
			static void call(int index, int time, int* details, ARGS... args)
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
		int   					scheduledtime_;
		int* 					details_;
};
