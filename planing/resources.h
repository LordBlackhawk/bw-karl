#pragma once

#include "tuple.h"
#include "restypes.h"

template <class GROWTHLIST, class depRT>
struct GrowthInverseList
{
  template <class RT>
  struct Predicate
  {
    enum { value = boost::is_same<depRT, Plan::ResourceGrowth<RT>::dependentRT >::value };
  };
  
  typedef sublist< Predicate, GROWTHLIST >::type type;
};

template <class RLIST>
class Resources
{
  typedef Resources<RLIST> ThisType;
  typedef sublist< Plan::ResourceLockable, RLIST > LOCKLIST;
  typedef sublist< Plan::ResourceGrowth, RLIST > GROWTHLIST;
  
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
       amount.get<RT>() += value;
       typedef GrowthInverseList< GROWTHLIST, RT >::type MYLIST;
       enumerate<MYLIST>::call<AdvanceInternal, Tuple<RLIST, int>&, int> (amount, time - optime);
     }
     
     template <class RT>
     void dec(int optime, int value = 1)
     {
       inc(optime, -value);
     }
	 
	   template <class RT>
     void incLocked(int optime, int value = 1)
	   {
        dec(optime, value);
        locked.get<RT>() += value;
     }
	 
     template <class RT>
     void decLocked(int optime, int value = 1)
     {
        incLocked(optime, -value);
     }
     
     void advance(int dt)
     {
       enumerate<GROWTHLIST>::call<AdvanceInternal, Tuple<RLIST, int>&, int> (amount, dt);
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
    template <class RT>
    struct AdvanceInternal
    {
      typedef Plan::ResourceGrowth<RT>::dependentRT depRT;
      enum { factor = Plan::ResourceGrowth<RT>::factor };
      static call(Tuple<RLIST, int>& amount, int dt)
      {
        amount.template get<RT>() += dt * factor * amount.template get<depRT>();
      }
    };

  protected:
    int                  time;
    Tuple<RLIST, int>    amount;
    Tuple<LOCKLIST, int> locked;
};
