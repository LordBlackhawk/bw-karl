#pragma once

#include "resources.h"
#include "optypes.h"

#include <set>
#include <string>
#include <stdexcept>

struct OperationStatus
{
	enum type { scheduled, running };
};

template <class RLIST, class OLIST>
class Operation
{
	public:
		 typedef Resources<RLIST>			ResourcesType;
		 typedef Operation<RLIST, OLIST>	ThisType;

	public:
		Operation(int i, int st = 0)
			: index_(i), status_(OperationStatus::scheduled), stage_(0), scheduledtime_(st), details_(NULL)
		{ }
		
		Operation(const ThisType& o, int st)
			: index_(o.index_), status_(o.status_), stage_(o.stage_), scheduledtime_(st), details_(o.details_)
		{ }
		
		template <class OP>
		static ThisType get(int st = 0)
		{
			return Operation(indexof<OP, OLIST>::value, st);
		}

		int duration() const
		{
			int result = 0;
			idispatch<CountDuration, int&>(result);
			return result;
		}

		/*
		int starttime() const
		{
			return starttime_;
		}

		int endtime() const
		{
			return starttime_ + duration();
		}
		*/
		
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

		/*
		bool isApplyable(const ResourcesType& res) const
		{
			bool result = true;
			disenum<IsApplyableInternal, const ResourcesType&, bool&>(res, result);
			return result;
		}

		void apply(ResourcesType& res, int btime, int etime) const
		{
			int time = 0;
 			disenum<ApplyInternal, ResourcesType&, int&, int, int, int>(res, time, time+duration(), btime-starttime_, etime-starttime_);
		}
		*/
      
      std::string getName() const
      {
        std::string result;
        dispatch<OLIST>::template call<GetName, std::string&>(index_, result);
        return result;
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
      
	private:
		template <class OT>
		struct GetName
		{
		  static void call(std::string& result)
		  {
			result = Plan::OperationName<OT>::name;
		  }
		};

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

	// private:
		// template <class OT, class T>
		// struct IsApplyableInternal
		// {
			// static void call(const ResourcesType& /*res*/, bool& /*result*/)
			// { }
		// };

		// template <class OT, int num, class RT>
		// struct IsApplyableInternal< OT, Needs<num, RT> >
		// {
			// static void call(const ResourcesType& res, bool& result)
			// {
				// if (res.template get<RT>() < num)
					// result = false;
			// }
		// };
      
		// template <class OT, int num, class RT>
		// struct IsApplyableInternal< OT, Locks<num, RT> >
		// {
			// static void call(const ResourcesType& res, bool& result)
			// {
				// if (res.template get<RT>() < num)
					// result = false;
			// }
		// };
      
		// template <class OT, int num, class RT>
		// struct IsApplyableInternal< OT, Consums<num, RT> >
		// {
			// static void call(const ResourcesType& res, bool& result)
			// {
				// if (res.template get<RT>() < num)
					// result = false;
			// }
		// };

	// private:
		// template <class OT, class T>
		// struct ApplyInternal
		// {
			// static void call(ResourcesType& /*res*/, int& /*time*/, int /*endtime*/, int /*btime*/, int /*etime*/)
			// { }
		// };

		// template <class OT, int num>
		// struct ApplyInternal< OT, Duration<num> >
		// {
			// static void call(ResourcesType& /*res*/, int& time, int /*endtime*/, int /*btime*/, int /*etime*/)
			// {
				// time += Plan::OperationDynamic< OT, Duration<num> >::getValue(details_);
			// }
		// };
      
		// template <class OT, int num, class RT>
		// struct ApplyInternal< OT, Consums<num, RT> >
		// {
			// static void call(ResourcesType& res, int& time, int /*endtime*/, int btime, int etime)
			// {
				// if ((btime <= time) && (time <= etime))
					// res.template get<RT>() -= num;
			// }
		// };

		// template <class OT, int num, class RT>
		// struct ApplyInternal< OT, Locks<num, RT> >
		// {
			// static void call(ResourcesType& res, int& time, int endtime, int btime, int etime)
			// {
				// if ((endtime < btime) || (etime < time))
					// return;
				// if ((time < btime) && (etime < endtime))
					// return;
 				// if ((btime <= time) && (endtime <= etime))
 					// return;

				// if ((btime <= time) && (time <= etime)) {
					// ! Nur mit "-". Beginn der Benutzung!
          // res.template get<RT>() -= num;
				// } else {
					// ! Nur mit "+". Ende der Benutzung!
					 // res.template get<RT>() += num;
				// }
			// }
		// };

		// template <class OT, int num, class RT>
		// struct ApplyInternal< OT, Prods<num, RT> >
		// {
			// static void call(ResourcesType& res, int& time, int /*endtime*/, int btime, int etime)
			// {
				// if ((time < btime) || (etime < time))
					// return;

				// res.template get<RT>() += num;
			// }
		// };

	private:
		template <class OT, class T>
		struct docall
		{
			template <template<class, class> class DISPATCHER, class... ARGS>
			static void call(int /*stage*/, int /*time*/, int* /*details*/, ARGS... /*args*/)
			{ }
		};
		
		template <class OT, class HT, int num>
		struct docall< OT, CheckPoint<HT, num> >
		{
			template <template<class, class> class DISPATCHER, class... ARGS>
			static void call(int /*stage*/, int& time, int* details, ARGS... /*args*/)
			{
				time += Plan::OperationDynamic< OT, CheckPoint<HT, num> >::getValue(details);
			}
		};
	
		template <class OT, class LIST>
		struct mydispatch2
		{
			template <template<class, class> class DISPATCHER, class... ARGS>
			static void call(int /*stage*/, int /*time*/, int* /*details*/, ARGS... /*args*/)
			{ }
		};
		
		template <class OT, class FIRST, class ... TAIL>
		struct mydispatch2< OT, type_list<FIRST, TAIL...> >
		{
			template <template<class, class> class DISPATCHER, class... ARGS>
			static void call(int stage, int time, int* details, ARGS... args)
			{
				docall<OT, FIRST>::template call<DISPATCHER, ARGS...>(stage, time, details, args...);
				if (stage >= 0)
					DISPATCHER<OT, FIRST>::call(time, details, args...);
				mydispatch2< OT, type_list<TAIL...> >::template call<DISPATCHER, ARGS...>(stage, time, details, args...);
			}
		};
		
		template <class LIST>
		struct mydispatch
		{
			template <template<class, class> class DISPATCHER, class ... ARGS>
			static void call(int /*index*/, int /*stage*/, int /*time*/, int* /*details*/, ARGS... /*args*/)
			{
				throw std::runtime_error("mydispatch<>::call() is called, but should not!");
			}
		};

		template <class FIRST, class ... TAIL>
		struct mydispatch< type_list<FIRST, TAIL...> >
		{
			template <template<class, class> class DISPATCHER, class ... ARGS>
			static void call(int index, int stage, int time, int* details, ARGS... args)
			{
				if (index == 0)
					mydispatch2< FIRST, typename Plan::OperationList<FIRST>::type >::template call<DISPATCHER, ARGS...>(stage, time, details, args...);
				else 
					mydispatch< type_list<TAIL...> >::template call<DISPATCHER, ARGS...>(index-1, stage, time, details, args...);
			}
		};
		
		template <template <class, class> class DISPATCHER, class ... ARGS>
		void idispatch(ARGS... args) const
		{
			mydispatch<OLIST>::template call<DISPATCHER, ARGS...>(index_, -stage_, scheduledtime_, details_, args...);
		}

	protected:
		int   					index_;
		OperationStatus::type 	status_;
		int						stage_;
		int   					scheduledtime_;
		int* 					details_;
};
