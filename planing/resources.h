#pragma once

#include "tuple.h"
#include "restypes.h"

template <class RLIST>
class Resources
{
  typedef Resources<RLIST> 											ThisType;
  typedef typename sublist< Plan::ResourceLockable, RLIST >::type 	LOCKLIST;
  typedef typename GrowthPairs<RLIST>::type 						PAIRLIST;
  
  public:
     Resources() : time(0), amount(0), locked(0)
     { }
     
     Resources(const Resources& r) : time(r.time), amount(r.amount), locked(r.locked)
     { }
	 
	 void swap(ThisType& other)
	 {
        amount.swap(other.amount);
        locked.swap(other.locked);
     }
	
     bool valid() const
     {
        for (int it : amount)
           if (it < 0)
              return false;
        for (int it : locked)
          if (it < 0)
            return false;
        return true;
     }
	 
	 template <class RT>
     int get() const
     {
       return amount.get<RT>();
     }
     
	 template <class RT>
     int getLocked() const
     {
       return locked.get<RT>();
     }
     
     template <class RT>
     int getExisting() const
     {
       return amount.get<RT>() + locked.get<RT>();
     }
     
     template <class RT>
     void set(int value)
     {
       amount.get<RT>() = value;
     }
     
     template <class RT>
     void setLocked(int value)
     {
       locked.get<RT>() = value;
     }
     
     template <class RT>
     void inc(int optime, int value = 1)
     {
		typedef typename GrowthInverseList< RLIST, RT >::type MYLIST;
		enumerate<MYLIST>::template call<IncInternal, Tuple<RLIST, int>&, int> (amount, value * (time - optime));
		amount.get<RT>() += value;
     }
     
     template <class RT>
     void dec(int optime, int value = 1)
     {
       inc<RT>(optime, -value);
     }
	 
	 template <class RT>
     void incLocked(int optime, int value = 1)
	 {
        dec<RT>(optime, value);
        locked.get<RT>() += value;
     }
	 
     template <class RT>
     void decLocked(int optime, int value = 1)
     {
        incLocked<RT>(optime, -value);
     }
     
     void advance(int dt)
     {
       enumerate<PAIRLIST>::template call<AdvanceInternal, Tuple<RLIST, int>&, int> (amount, dt);
       time += dt;
     }
     
     void setTime(int newtime)
     {
       time = newtime;
     }
     
     int getTime() const
     {
       return time;
     }
     
  private:
    template <class PT>
	struct IncInternal
	{
		typedef typename PT::RT RT;
		enum { factor = PT::factor };
		static void call(Tuple<RLIST, int>& amount, int dt_value)
		{
			amount.template get<RT>() += dt_value * factor;
		}
	};
	
    template <class PT>
    struct AdvanceInternal
    {
		typedef typename PT::RT RT;
		typedef typename PT::depRT depRT;
		enum { factor = PT::factor };
		static void call(Tuple<RLIST, int>& amount, int dt)
		{
			amount.template get<RT>() += dt * factor * amount.template get<depRT>();
		}
    };

  protected:
    int                  time;
    Tuple<RLIST, int>    amount;
    Tuple<LOCKLIST, int> locked;
};
