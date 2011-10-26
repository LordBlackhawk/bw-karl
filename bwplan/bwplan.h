#pragma once

#include "planing/plan.h"

#include "user-res-types.h"
#include "auto-res-types.h"
#include "user-op-types.h"
#include "auto-op-types.h"

typedef combine< UserResourceList, AutoResourceList >    BWResourceList;
typedef combine< UserOperationList, AutoOpertaionList >  BWOperationList;

typedef PlanContainer<res_list, op_list>                 BWPlan;
typedef BWPlan::ResourcesType                            BWResources;
typedef BWPlan::OperationType                            BWOperation;

