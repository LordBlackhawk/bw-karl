#pragma once

#include "planing/plan.h"
#include "planing/association.h"
#include "planing/simplerequirements.h"

#ifndef NO_ASSOCIATIONS
#include <BWAPI.h>
#endif

#include "user-res-types.h"
#include "auto-res-types.h"
#include "user-op-types.h"
#include "auto-op-types.h"

typedef TL::combine< UserResourceTypeList, AutoResourceTypeList >::type		BWResourceList;
typedef TL::combine< UserOperationTypeList, AutoOperationTypeList >::type	BWOperationList;

typedef PlanContainer<BWResourceList, BWOperationList>              		BWPlan;
typedef BWPlan::ResourcesType                            					BWResources;
typedef BWPlan::OperationType                            					BWOperation;
typedef BWOperation::IndexType							 					BWOperationIndex;
typedef BWOperation::ResIndexType						 					BWResourceIndex;
