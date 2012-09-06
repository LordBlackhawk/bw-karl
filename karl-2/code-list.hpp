#pragma once

#include "code-caller.hpp"
#include "message-dispatch.hpp"
#include "mineral-line.hpp"
#include "resources.hpp"
#include "supply.hpp"
#include "building-placer.hpp"
#include "idle-unit-container.hpp"
#include "unit-morpher.hpp"
#include "unit-builder.hpp"
#include "zerg-strategie.hpp"
#include "hud-code.hpp"
#include "debugger.hpp"

typedef CodeCaller<
					MessageDispatchCode,
                    MineralLineCode,
					ResourcesCode,
					SupplyCode,
					IdleUnitContainerCode,
					BuildingPlacerCode,
					UnitBuilderCode,
					UnitMorpherCode,
					HUDCode,
					ZergStrategieCode,
				//	DebuggerCode,
					DefaultCode
                  >
        CodeList;
