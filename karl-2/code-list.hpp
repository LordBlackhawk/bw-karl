#pragma once

#include "code-caller.hpp"
#include "message-dispatch.hpp"
#include "mineral-line.hpp"
#include "resources.hpp"
#include "supply.hpp"
#include "idle-unit-container.hpp"
#include "unit-morpher.hpp"

typedef CodeCaller<
					MessageDispatchCode,
                    MineralLineCode,
					ResourcesCode,
					SupplyCode,
					IdleUnitContainerCode,
					UnitMorpherCode
                  >
        CodeList;
