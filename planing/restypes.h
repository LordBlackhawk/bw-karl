#pragma once

#include "typelists.h"

#include <string>

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
	struct Name { };                                                           \
	template <> const char Plan::ResourceName<Name>::name[] = #Name;
   
#define DEF_RESGROWTH(Name)                                                    \
  namespace Plan { template <> struct ResourceGrowth<Name>                     \
  {                                                                            \
    enum { isgrowing = true };                                                 \
	template <class RESTYPE>                                                   \
    static int getGrowth(const RESTYPE& res);                                  \
  }; }                                                                         \
  template <class RESTYPE>                                                     \
  int Plan::ResourceGrowth<Name>::getGrowth(const RESTYPE& res)
  
#define DEF_RESSTOPTIME(Name)                                                  \
  namespace Plan { template <> struct ResourceStopTime<Name>                   \
  {                                                                            \
    enum { needstoptime = true };                                              \
	template <class RESTYPE>                                                   \
    static int getStopTime(const RESTYPE& res);                                \
  }; }                                                                         \
  template <class RESTYPE>                                                     \
  int Plan::ResourceStopTime<Name>::getStopTime(const RESTYPE& res)
  
template <class RLIST>
class ResourceIndex
{
  typedef ResourceIndex<RLIST> ThisType;

  public:
    ResourceIndex(int i) : index_(i)
    { }
    
    template <class RT>
    static ThisType get()
    {
      return ResourceIndex(indexof<RT, RLIST>::value);
    }
    
    std::string getName() const
    {
      std::string result;
      dispatch<RLIST>::template call<GetName, std::string&>(index_, result);
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
