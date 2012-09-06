#pragma once

#include "code-caller.hpp"
#include "message-dispatch.hpp"
#include "mineral-line.hpp"
#include "resources.hpp"
#include "supply.hpp"
#include "building-placer.hpp"
#include "idle-unit-container.hpp"
#include "unit-morpher.hpp"
#include "hud-code.hpp"
#include "debugger.hpp"

typedef CodeCaller<
					MessageDispatchCode,
                    MineralLineCode,
					ResourcesCode,
					SupplyCode,
					IdleUnitContainerCode,
					BuildingPlacerCode,
					UnitMorpherCode,
					HUDCode,
				//	DebuggerCode,
					DefaultCode
                  >
        CodeList;
