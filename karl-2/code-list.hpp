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
#include "larvas.hpp"
#include "requirements.hpp"

typedef CodeCaller<
					MessageDispatchCode,
                    MineralLineCode,
					ResourcesCode,
					IdleUnitContainerCode,
					LarvaCode,
					RequirementsCode,
					BuildingPlacerCode,
					UnitBuilderCode,
					UnitMorpherCode,
					SupplyCode,
					HUDCode,
					ZergStrategieCode,
					DefaultCode
                  >
        CodeList;
