#pragma once

#include "tuple.h"
#include "restypes.h"

template <class RLIST>
class Resources
{
  public:
     Resources() : amount(0), locked(0)
     { }
     
     Resources(const Resources& r) : amount(r.amount), locked(r.locked)
     { }
	
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
     int& get()
     {
       return amount.get<RT>();
     }
	 
	 template <class RT>
     int get() const
     {
       return amount.get<RT>();
     }
     
     template <class RT>
     int& getLocked()
     {
       return locked.get<RT>();
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
	 void incLocked(int value = 1)
	 {
		amount.get<RT> -= 1;
		locked.get<RT> += 1;
	 }
	 
	 template <class RT>
	 void decLocked(int value = 1)
	 {
		amount.get<RT> += 1;
		locked.get<RT> -= 1;
	 }
     
     int& get(const ResourceIndex<RLIST>& ri)
     {
       return amount[ri.getIndex()];
     }
     
     int& getLocked(const ResourceIndex<RLIST>& ri)
     {
       return locked[ri.getIndex()];
     }
     
     int getExisting(const ResourceIndex<RLIST>& ri) const
     {
       return amount[ri.getIndex()] + locked[ri.getIndex()];
     }
     
  protected:
    Tuple<RLIST, int> amount;
    Tuple<RLIST, int> locked;
};
