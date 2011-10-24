#pragma once

#include "resources.h"

namespace Plan
{
	template <class RT>
	struct ResourceName
	{
		static const char name[];
	};
  
  template <class RT>
  struct ResourceGrowth
  {
    enum { isgrowing = false };
  };
  
  template <class RT>
  struct ResourceStopTime
  {
    enum { needstoptime = false };
  };
}
	
#define DEF_RESTYPE(Name)                                                      \
	struct Name { };                                                             \
	template <> const char Plan::ResourceName<Name>::name[] = #Name;
   
#define DEF_RESGROWTH(Name)                                                    \
  template <> struct Plan::ResourceGrowth<Name>                                \
  {                                                                            \
    enum { isgrowing = true };                                                 \
    static int getGrowth(const Resources& res);                                \
  };                                                                           \
  template <> static int Plan::ResourceGrowth<Name>::getGrowth(const Resources& res)
  
#define DEF_RESSTOPTIME(Name)                                                  \
  template <> struct Plan::ResourceStopTime<Name>                              \
  {                                                                            \
    enum { needstoptime = true };                                              \
    static int getStopTime(const Resources& res);                              \
  };                                                                           \
  template <> static int Plan::ResourceStopTime<Name>::getStopTime(const Resources& res)
  
template <RLIST>
class ResourceIndex
{
  typedef ResourceIndex<RLIST> ThisType;

  public:
    ResourceIndex(int i) : index_(i)
    { }
    
    template <RT>
    static ThisType get()
    {
      return ResourceIndex(indexof<RT, RLIST>::value);
    }
    
    std::string getName() const
    {
      std::string result;
      dispatch<RLIST>::call<GetName, std::string&>(index_, result);
      return result;
    }
    
    int getIndex() const
    {
      return index_;
    }
    
  private:
    template <class RT>
    struct GetName
    {
      static void call(std::string& result)
      {
        result = Plan::ResourceName<RT>::name;
      }
    };
    
  protected:
    int index_;
};
