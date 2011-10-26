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
  struct ResourceLockable
  {
    enum { value = false };
  };
  
  template <class RT>
  struct ResourceGrowth
  {
    enum { value = false };
  };
}
 
#define DEF_RESTYPE(Name)                                                      \
	struct Name { };                                                             \
	template <> const char Plan::ResourceName<Name>::name[] = #Name;

#define DEF_RESLOCKABLE(Name)                                                  \
  struct Name;                                                                 \
  namespace Plan { template <> struct ResourceLockable<Name>                   \
  { enum { value = true }; }; }

#define DEF_RESGROWTH(Name, Factor, DepName)                                   \
  struct Name;                                                                 \
  struct DepName;                                                              \
  namespace Plan { template <> struct ResourceGrowth<Name>                     \
  {                                                                            \
    enum { value = true };                                                     \
    enum { factor = Factor };                                                  \
    typedef DepName dependentRT;                                               \
  }; }

