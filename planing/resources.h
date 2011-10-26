#pragma once

#include "tuple.h"
#include "restypes.h"

template <class RLIST>
class Resources
{
  typedef Resources<RLIST> ThisType;
  typedef sublist< Plan::ResourceLockable, RLIST > LOCKLIST;
  
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
       // TODO: ResGrowth
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
       // TODO: ResGrowth
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

  protected:
    int                  time;
    Tuple<RLIST, int>    amount;
    Tuple<LOCKLIST, int> locked;
};
