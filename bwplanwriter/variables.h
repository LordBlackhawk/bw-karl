#pragma once

#include <map>
#include <vector>
#include <BWAPI.h>

struct ResourceDescription;
struct OperationDescription;

std::vector<ResourceDescription*> resourceDescriptions;
std::map<BWAPI::UnitType, ResourceDescription*> unitMap;
std::map<BWAPI::Race, ResourceDescription*> workerMap;
std::map<BWAPI::Race, ResourceDescription*> gasworkerMap;
std::map<BWAPI::Race, ResourceDescription*> supplyMap;
std::map<BWAPI::TechType, ResourceDescription*> techMap;
std::map<BWAPI::UpgradeType, ResourceDescription*> upgradeMap;
ResourceDescription* minerals;
ResourceDescription* gas;
ResourceDescription* workingplaces;
ResourceDescription* larva;

std::vector<OperationDescription*> operationDescriptions;
