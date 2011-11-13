#pragma once

#include "planing/plan.h"
#include "planing/association.h"
#include "planing/simplerequirements.h"

#ifndef NO_ASSOCIATIONS
#include <BWAPI.h>
#endif

#include "auto-res-types.h"
#include "user-res-types.h"
#include "user-checkpoints.h"
#include "auto-op-types.h"
#include "user-op-types.h"
#include "user-names.h"
#include "user-linear-correction.h"

struct BWTraits
{
	typedef BWTraits															Traits;
	typedef TL::combine< UserResourceTypeList, AutoResourceTypeList >::type		ResourceList;
	typedef TL::combine< UserOperationTypeList, AutoOperationTypeList >::type	OperationList;
	typedef BWUserNames<Traits>													NameTraits;
	typedef UserLinearCorrectionHandler<Traits>									CorrectionTraits;
};

typedef PlanContainer<BWTraits>              		BWPlan;
typedef BWPlan::ResourcesType                    	BWResources;
typedef BWResources::IndexType						BWResourceIndex;
typedef BWResourceIndex::ContainerType				BWResourceIndexContainer;
typedef BWPlan::OperationType                       BWOperation;
typedef BWOperation::IndexType						BWOperationIndex;
typedef BWOperationIndex::ContainerType				BWOperationIndexContainer;

BWResourceIndexContainer 	BWResourceIndices;
BWOperationIndexContainer	BWOperationIndices;