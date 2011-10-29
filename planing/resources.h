#pragma once

#include "resourceindex.h"

#include <array>

template <class RLIST>
class Resources
{
  typedef Resources<RLIST> 												ThisType;
  typedef ResourceIndex<RLIST>											IndexType;
  
  typedef std::array<int, IndexType::IndexCount>						AmountType;
  typedef std::array<int, IndexType::LockedIndexCount>					LockedType;

  typedef typename GrowthPairs<RLIST>::type 							PairTypeList;
  
  public:	
    Resources() : time(0)
    { 
		for (auto& it : amount)
			it = 0;
		for (auto& it : locked)
			it = 0;
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
	 
  public:
	int get(const IndexType& ri) const
	{
		return amount[ri.getIndex()];
	}
	
	int getLocked(const IndexType& ri) const
	{
		return locked[ri.getLockedIndex()];
	}
	
	int getExisting(const IndexType& ri) const
	{
		return get(ri) + getLocked(ri);
	}
	
	void set(const IndexType& ri, int value)
	{
		amount[ri.getIndex()] = value;
	}
	
	void setLocked(const IndexType& ri, int value)
	{
		locked[ri.getLockedIndex()] = value;
	}
	
	void inc(const IndexType& ri, int optime, int value)
	{
		TL::dispatch<RLIST>::template call<IncInternal, AmountType&, int> (ri.getIndex(), amount, value * (time - optime));
		amount[ri.getIndex()] += value;
	}
	
	void dec(const IndexType& ri, int optime, int value)
	{
		inc(ri, optime, -value);
	}
	
	void incLocked(const IndexType& ri, int optime, int value)
	{
		dec(ri, optime, value);
		locked[ri.getLockedIndex()] += value;
	}
	
	void decLocked(const IndexType& ri, int optime, int value)
	{
		incLocked(ri, optime, -value);
	}

  public:
	template <class RT>
    int get() const
    {
		return get(IndexType::template byClass<RT>());
    }
     
	template <class RT>
    int getLocked() const
    {
       return getLocked(IndexType::template byClass<RT>());
    }
     
    template <class RT>
    int getExisting() const
	{
       return getExisting(IndexType::template byClass<RT>());
    }
     
    template <class RT>
    void set(int value)
    {
		set(IndexType::template byClass<RT>(), value);
    }
     
    template <class RT>
    void setLocked(int value)
    {
		setLocked(IndexType::template byClass<RT>(), value);
    }
     
    template <class RT>
    void inc(int optime, int value = 1)
    {
		inc(IndexType::template byClass<RT>(), optime, value);
    }
     
    template <class RT>
    void dec(int optime, int value = 1)
    {
		dec(IndexType::template byClass<RT>(), optime, value);
    }
	 
	template <class RT>
    void incLocked(int optime, int value = 1)
	{
        incLocked(IndexType::template byClass<RT>(), optime, value);
    }
	 
    template <class RT>
    void decLocked(int optime, int value = 1)
    {
        decLocked(IndexType::template byClass<RT>(), optime, value);
    }
    
  public:
    void advance(int dt)
    {
       TL::enumerate<PairTypeList>::template call<AdvanceInternal, AmountType&, int> (amount, dt);
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
	struct IncInternal2
	{
		typedef typename PT::RT RT;
		enum { factor = PT::factor };
		static void call(AmountType& amount, int dt_value)
		{
			IndexType ri = IndexType::template byClass<RT>();
			amount[ri.getIndex()] += dt_value * factor;
		}
	};
	
	template <class RT>
	struct IncInternal
	{
		typedef typename GrowthInverseList< RLIST, RT >::type MYLIST;
		static void call(AmountType& amount, int dt_value)
		{
			TL::enumerate<MYLIST>::template call<IncInternal2, AmountType&, int> (amount, dt_value);
		}
	};
	
    template <class PT>
    struct AdvanceInternal
    {
		typedef typename PT::RT RT;
		typedef typename PT::depRT depRT;
		enum { factor = PT::factor };
		static void call(AmountType& amount, int dt)
		{
			IndexType ri    = IndexType::template byClass<RT>();
			IndexType depri = IndexType::template byClass<depRT>();
			amount[ri.getIndex()] += dt * factor * amount[depri.getIndex()];
		}
    };

  protected:
    int             time;
	AmountType		amount;
	LockedType		locked;
};
