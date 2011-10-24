#pragma once

#include "resources.h"
#include "optypes.h"

#include <set>
#include <string>

template <class RLIST, class OLIST>
class Operation
{
	public:
		 typedef Resources<RLIST>			ResourcesType;
		 typedef Operation<RLIST, OLIST>	ThisType;

	public:
		Operation(int i, int st = 0)
			: index(i), starttime_(st), details_(NULL)
		{ }
		
		Operation(const ThisType& o, int st)
			: index(o.index), starttime_(st), details_(o.details_)
		{ }
		
		template <class OP>
		static ThisType get(int st = 0)
		{
			return Operation(indexof<OP, OLIST>::value, st);
		}

		int duration() const
		{
			int result = 0;
			disenum<CountDuration, int&>(result);
			return result;
		}

		int starttime() const
		{
			return starttime_;
		}

		int endtime() const
		{
			return starttime_ + duration();
		}
		
		void changeTimes(std::set<int>& result) const
		{
			int time = starttime_;
			result.insert(time);
			disenum<ChangeTimesInternal, std::set<int>&, int&>(result, time);
		}
		
		std::set<int> changeTimes() const
		{
			std::set<int> result;
			changeTimes(result);
			return result;
		}

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
      
      std::string getName() const
      {
        std::string result;
        dispatch<OLIST>::template call<GetName, std::string&>(index, result);
        return result;
      }
      
      template <OT>
      Plan::OperationDetails<OT>::type* getDetails()
      {
        return (Plan::OperationDetails<OT>::type*) details_;
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
			static void call(int& /*result*/)
			{ }
		};

		template <class OT, int num>
		struct CountDuration< OT, Duration<num> >
		{
			static void call(int& result)
			{
				result += Plan::OperationDynamic< OT, Duration<num> >::getValue(details_);
			}
		};
		
	private:
		template <class OT, class T>
		struct ChangeTimesInternal
		{
			static void call(std::set<int>& /*timeSet*/, int& /*time*/)
			{ }
		};
		
		template <class OT, int num>
		struct ChangeTimesInternal< OT, Duration<num> >
		{
			static void call(std::set<int>& timeSet, int& time)
			{
				time += Plan::OperationDynamic< OT, Duration<num> >::getValue(details_);
				timeSet.insert(time);
			}
		};

	private:
		template <class OT, class T>
		struct IsApplyableInternal
		{
			static void call(const ResourcesType& /*res*/, bool& /*result*/)
			{ }
		};

		template <class OT, int num, class RT>
		struct IsApplyableInternal< OT, Needs<num, RT> >
		{
			static void call(const ResourcesType& res, bool& result)
			{
				if (res.template get<RT>() < num)
					result = false;
			}
		};
      
     template <class OT, int num, class RT>
		struct IsApplyableInternal< OT, Locks<num, RT> >
		{
			static void call(const ResourcesType& res, bool& result)
			{
				if (res.template get<RT>() < num)
					result = false;
			}
		};
      
     template <class OT, int num, class RT>
		struct IsApplyableInternal< OT, Consums<num, RT> >
		{
			static void call(const ResourcesType& res, bool& result)
			{
				if (res.template get<RT>() < num)
					result = false;
			}
		};

	private:
		template <class OT, class T>
		struct ApplyInternal
		{
			static void call(ResourcesType& /*res*/, int& /*time*/, int /*endtime*/, int /*btime*/, int /*etime*/)
			{ }
		};

		template <class OT, int num>
		struct ApplyInternal< OT, Duration<num> >
		{
			static void call(ResourcesType& /*res*/, int& time, int /*endtime*/, int /*btime*/, int /*etime*/)
			{
				time += Plan::OperationDynamic< OT, Duration<num> >::getValue(details_);
			}
		};
      
     template <class OT, int num, class RT>
		struct ApplyInternal< OT, Consums<num, RT> >
		{
			static void call(ResourcesType& /*res*/, int& time, int /*endtime*/, int /*btime*/, int /*etime*/)
			{
				if ((btime <= time) && (time <= etime))
					res.template get<RT>() -= num;
			}
		};

		template <class OT, int num, class RT>
		struct ApplyInternal< OT, Locks<num, RT> >
		{
			static void call(ResourcesType& res, int& time, int endtime, int btime, int etime)
			{
				if ((endtime < btime) || (etime < time))
					return;
				if ((time < btime) && (etime < endtime))
					return;
 				if ((btime <= time) && (endtime <= etime))
 					return;

				if ((btime <= time) && (time <= etime)) {
					//! Nur mit "-". Beginn der Benutzung!
          res.template get<RT>() -= num;
				} else {
					//! Nur mit "+". Ende der Benutzung!
					 res.template get<RT>() += num;
				}
			}
		};

		template <class OT, int num, class RT>
		struct ApplyInternal< OT, Prods<num, RT> >
		{
			static void call(ResourcesType& res, int& time, int /*endtime*/, int btime, int etime)
			{
				if ((time < btime) || (etime < time))
					return;

				res.template get<RT>() += num;
			}
		};

	private:
		template <class LIST>
		struct mydispatch
		{
			template <template<class> class ENUMERATOR, class ... ARGS>
			static void call(int /*index*/, ARGS... /*args*/)
			{ }
		};

		template <class F, class ... T>
		struct mydispatch< type_list<F, T...> >
		{
			template <template<class, class> class ENUMERATOR, class ... ARGS>
			static void call(int index, ARGS... args)
			{
        template <class TT> using MYENUM = ENUMERATOR<F, TT>;
				if (index == 0)
					enumerate< Plan::OperationList<F>::type >::template call<MYENUM, ARGS...>(args...);
				mydispatch< type_list<T...> >::template call<ENUMERATOR, ARGS...>(index-1, args...);
			}
		};
		
		template <template <class> class DISENUMOR, class ... ARGS>
		void disenum(ARGS... args) const
		{
			mydispatch<OLIST>::template call<DISENUMOR, ARGS...>(index, args...);
		}

	protected:
		int   index;
		int   starttime_;
    void* details_;
};
