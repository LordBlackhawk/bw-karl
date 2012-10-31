#pragma once

#include "code-caller.hpp"
#include "log.hpp"
#include "message-dispatch.hpp"
#include "mineral-line.hpp"
#include "resources.hpp"
#include "supply.hpp"
#include "building-placer.hpp"
#include "idle-unit-container.hpp"
#include "unit-morpher.hpp"
#include "unit-builder.hpp"
#include "unit-trainer.hpp"
#include "zerg-strategie.hpp"
#include "terran-strategie.hpp"
#include "hud-code.hpp"
#include "larvas.hpp"
#include "requirements.hpp"
#include "tech-upgrades.hpp"

typedef CodeCaller<
					LogCode,
					MessageDispatchCode,
                    MineralLineCode,
					ResourcesCode,
					IdleUnitContainerCode,
					LarvaCode,
					RequirementsCode,
					BuildingPlacerCode,
					UnitBuilderCode,
					UnitMorpherCode,
					UnitTrainerCode,
					TechCode,
					SupplyCode,
					HUDCode,
					ZergStrategieCode,
					TerranStrategieCode,
					DefaultCode
                  >
        CodeList;
