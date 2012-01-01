#include "resourcedescription.h"
#include "operationdescription.h"

#include <BWAPI.h>
#include <boost/format.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>

void writeHeader()
{
	std::cout << "// This file is auto generated by BWPlanWriter.\n\n";
}

std::string removeSpaces(const std::string text)
{
	std::string result = text;
	int c = std::count(result.begin(), result.end(), ' ');
	std::remove(result.begin(), result.end(), ' ');
	result.resize(result.size()-c);
	std::replace(result.begin(), result.end(), '-', '_');
	return result;
}

std::string toCName(const std::string text)
{
	std::string result = text;
	std::replace(result.begin(), result.end(), ' ', '_');
	std::replace(result.begin(), result.end(), '-', '_');
	return result;
}

std::string getResourceName(const BWAPI::UnitType& ut)
{
	return removeSpaces(ut.getName());
}

std::string getOperationName(const BWAPI::UnitType& ut)
{
	return removeSpaces(ut.getName());
}

std::string getTechName(const BWAPI::TechType& tt)
{
	return removeSpaces(tt.getName());
}

std::string getUpgradeResName(const BWAPI::UpgradeType& gt)
{
	return removeSpaces(gt.getName());
}

std::string getUpgradeName(const BWAPI::UpgradeType& gt, int level)
{
	return str( boost::format("%s%d") % removeSpaces(gt.getName()) % level ); ;
}

template <class T>
void writeStringList(const std::vector<T*>& list, const std::string addIndex = "")
{
	writeHeader();
	std::cout << "enum Type {\n";
	bool first = true;
	for (auto it : list) {
		if (!first)
			std::cout << ",\n";
		std::cout << "\t\t" << it->name;
		first = false;
	}
	std::cout << ",\n\n\t\tIndexEnd,\n\t\tIndexBegin = " << (*list.begin())->name;
	std::cout << addIndex;
	std::cout << ",\n\t\tNone = -1";
	std::cout << "\n\t};\n";
}

bool isResource(const BWAPI::UnitType& ut)
{
	if (!ut.isBuilding())
		return false;
	if (ut.isHero())
		return false;
	if (ut.isSpecialBuilding())
		return false;
	if (ut.isMineralField())
		return false;
	if (ut == BWAPI::UnitTypes::Resource_Vespene_Geyser)
		return false;
	if (ut.isRefinery())
		return false;
	if (ut == BWAPI::UnitTypes::Terran_Missile_Turret)
		return false;
	if (ut == BWAPI::UnitTypes::Zerg_Nydus_Canal)
		return false;
	if (ut == BWAPI::UnitTypes::Protoss_Shield_Battery)
		return false;
	if (ut == BWAPI::UnitTypes::Protoss_Photon_Cannon)
		return false;
	if (ut == BWAPI::UnitTypes::Zerg_Creep_Colony)
		return false;
	if (ut == BWAPI::UnitTypes::Zerg_Spore_Colony)
		return false;
	if (ut == BWAPI::UnitTypes::Zerg_Sunken_Colony)
		return false;
	if (ut == BWAPI::UnitTypes::Terran_Supply_Depot)
		return false;
	if (ut == BWAPI::UnitTypes::Protoss_Pylon)
		return false;
	if (ut == BWAPI::UnitTypes::Terran_Bunker)
		return false;
	return true;
}

void writeResForUnitType(const BWAPI::UnitType& ut)
{
	if (!isResource(ut))
		return;

	ResourceDescription* res = new ResourceDescription(getResourceName(ut), true);
	res->race = ut.getRace();
	res->ut   = ut;
	unitMap[ut] = res;
}

bool isTechResource(const BWAPI::TechType& tt)
{
	BWAPI::UnitType what = tt.whatResearches();
	if (what == BWAPI::UnitTypes::None)
		return false;
	if (tt != BWAPI::TechTypes::Lurker_Aspect)
		return false;
	return true;
}

void writeResForTechType(const BWAPI::TechType& tt)
{
	if (!isTechResource(tt))
		return;

	ResourceDescription* res = new ResourceDescription(getTechName(tt), false);
	res->race = tt.getRace();
	res->tt   = tt;
	techMap[tt] = res;
}

bool isOperation(const BWAPI::UnitType& ut)
{
	if (ut.isHero())
		return false;
	if (ut.isSpecialBuilding())
		return false;
	if (ut.isMineralField())
		return false;
	if (ut == BWAPI::UnitTypes::Resource_Vespene_Geyser)
		return false;
	if (ut.getRace() != BWAPI::Races::Zerg && ut.getRace() != BWAPI::Races::Protoss && ut.getRace() != BWAPI::Races::Terran)
		return false;
	if ((ut.mineralPrice() == 1) && (ut.gasPrice() == 1))
		return false;
	if (ut.isSpell())
		return false;
	if (ut == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode)
		return false;
	if (ut == BWAPI::UnitTypes::Protoss_Dark_Archon)
		return false;
	if (ut == BWAPI::UnitTypes::Protoss_Archon)
		return false;
	if (ut == BWAPI::UnitTypes::Protoss_Interceptor)
		return false;
	if (ut == BWAPI::UnitTypes::Protoss_Scarab)
		return false;
	//if (ut == BWAPI::UnitTypes::Zerg_Lurker)
	//	return false;
	if (ut == BWAPI::UnitTypes::Terran_Vulture_Spider_Mine)
		return false;
	return true;
}

void writeOpForUnitType(const BWAPI::UnitType& ut, OperationDescription* op = NULL)
{
	if (!isOperation(ut))
		return;
		
	bool isnew = (op == NULL);
	if (isnew) {
		op = new OperationDescription(getOperationName(ut));
		
		op->ut = ut;
		op->race = ut.getRace();
		
		if (isResource(ut))
			unitMap[ut]->associated.push_back(op);
		if (ut == BWAPI::UnitTypes::Zerg_Hatchery)
			larva->associated.push_back(op);
		if (ut == BWAPI::UnitTypes::Zerg_Overlord)
			supplyMap[BWAPI::Races::Zerg]->associated.push_back(op);
		if (ut == BWAPI::UnitTypes::Protoss_Pylon)
			supplyMap[BWAPI::Races::Protoss]->associated.push_back(op);
		if (ut == BWAPI::UnitTypes::Terran_Supply_Depot)
			supplyMap[BWAPI::Races::Terran]->associated.push_back(op);
		if (ut.isWorker())
			minerals->associated.push_back(op);
	}
	
	// Morph required units before morphing this unit.
	if ((ut != BWAPI::UnitTypes::Zerg_Lair) && (ut != BWAPI::UnitTypes::Zerg_Hive))
		for (auto it : ut.requiredUnits())
			if (!isResource(it.first) && (it.first != BWAPI::UnitTypes::Zerg_Larva) && (!it.first.isWorker()))
				writeOpForUnitType(it.first, op);

	if (ut.isBuilding()) {
		if (ut.isAddon()) {
			auto what = ut.whatBuilds();
			op->locks(1, unitMap[what.first]);
		} else if (ut == BWAPI::UnitTypes::Zerg_Lair) {
			op->consums(1, unitMap[BWAPI::UnitTypes::Zerg_Hatchery]);
		} else if (ut == BWAPI::UnitTypes::Zerg_Hive) {
			op->locks(1, unitMap[BWAPI::UnitTypes::Zerg_Lair]);
		} else if ((ut != BWAPI::UnitTypes::Zerg_Sunken_Colony) && (ut != BWAPI::UnitTypes::Zerg_Spore_Colony)) {
			op->locks(1, workerMap[ut.getRace()]);
			op->checkpoint("CSendWorkerToBuildingPlace", 45);
			if (ut.getRace() == BWAPI::Races::Zerg) {
				op->unlocks(1, workerMap[ut.getRace()]);
				op->consums(1, workerMap[ut.getRace()]);
				op->unlocks(2, supplyMap[ut.getRace()]);
			}
		}
	}
	for (auto it : ut.requiredUnits())
		if (isResource(it.first))
			op->needs(it.second, unitMap[it.first]);

	if (ut.mineralPrice() > 0)
		op->consums(ut.mineralPrice(), minerals);
	if (ut.gasPrice() > 0)
		op->consums(ut.gasPrice(), gas);
	if (ut.supplyRequired() > 0)
		op->locks((ut.isTwoUnitsInOneEgg() ? 2 : 1) * ut.supplyRequired(), supplyMap[ut.getRace()]);
	if (ut.requiredTech() != BWAPI::TechTypes::None)
		op->needs(1, techMap[ut.requiredTech()]);
	if (ut.isBuilding()) {
		if (ut.isAddon()) {
			auto what = ut.whatBuilds();
			op->checkpoint("CBuildAddon", ut.buildTime());
			op->unlocks(1, unitMap[what.first]);
			op->checkpoint("CBuildAddonFinished", 1);
		} else {
			if (ut.getRace() == BWAPI::Races::Protoss)
				op->unlocks(1, workerMap[BWAPI::Races::Protoss]);
			op->checkpoint("CBuildBuilding", ut.buildTime());
			if (isResource(ut))
				op->prods(1, unitMap[ut]);
			if (ut.getRace() == BWAPI::Races::Terran)
				op->unlocks(1, workerMap[BWAPI::Races::Terran]);
			if (ut.supplyProvided() > 0)
				op->prods(ut.supplyProvided(), supplyMap[ut.getRace()]);
			if (ut.isRefinery()) {
				op->prods(3, workingplaces);
				workingplaces->associated.push_back(op);
			}
			if (ut == BWAPI::UnitTypes::Zerg_Hive)
				op->unlocks(1, unitMap[BWAPI::UnitTypes::Zerg_Lair]);
			op->checkpoint("CBuildingFinished", 1);
		}
	} else {
		bool writeUnitFinished = false;
		auto what = ut.whatBuilds();
		if (what.first == BWAPI::UnitTypes::Zerg_Larva) {
			op->consums(1, larva);
			op->checkpoint("CMorphUnit", ut.buildTime());
		} else if (ut.getRace() == BWAPI::Races::Zerg) {
			op->checkpoint("CMorphUnit", ut.buildTime());
		} else if (what.second != 1) {
			op->consums(what.second, unitMap[what.first]);
		} else {
			op->locks(1, unitMap[what.first]);
			op->checkpoint("CTrainUnit", ut.buildTime());
			op->unlocks(1, unitMap[what.first]);
			writeUnitFinished = true;
		}
		if (ut.isWorker()) {
			op->prods(1, workerMap[ut.getRace()]);
			writeUnitFinished = true;
		}
		if (ut.supplyProvided() > 0) {
			op->prods(ut.supplyProvided(), supplyMap[ut.getRace()]);
			writeUnitFinished = true;
		}
		if (!isnew)
			writeUnitFinished = true;
		if (writeUnitFinished)
			op->checkpoint("CUnitFinished", 1);
	}
}

void writeOpForTechType(const BWAPI::TechType& tt)
{
	BWAPI::UnitType what = tt.whatResearches();
	if (what == BWAPI::UnitTypes::None)
		return;
	
	OperationDescription* op = new OperationDescription(getTechName(tt));
	op->race = tt.getRace();
	op->tt = tt;
	
	if (isTechResource(tt))
		techMap[tt]->associated.push_back(op);
	
	if (tt == BWAPI::TechTypes::Lurker_Aspect)
		op->needs(1, unitMap[BWAPI::UnitTypes::Zerg_Lair]);
	if (tt.mineralPrice() > 0)
		op->consums(tt.mineralPrice(), minerals);
	if (tt.gasPrice() > 0)
		op->consums(tt.gasPrice(), gas);
	op->locks(1, unitMap[what]);
	op->checkpoint("CTechStart", tt.researchTime());
	if (isTechResource(tt))
		op->prods(1, techMap[tt]);
	op->unlocks(1, unitMap[what]);
	op->checkpoint("CTechFinished", 1);
}

void writeOpForUpgradeType(const BWAPI::UpgradeType& gt)
{
	BWAPI::UnitType what = gt.whatUpgrades();
	if (what == BWAPI::UnitTypes::None)
		return;
		
	ResourceDescription* res = new ResourceDescription(getUpgradeResName(gt));
	res->race = gt.getRace();
	res->gt   = gt;
	
	for (int k=1; k<=gt.maxRepeats(); ++k)
	{
		OperationDescription* op = new OperationDescription(getUpgradeName(gt, k));
		op->race = gt.getRace();
		op->gt   = gt;
		
		// Bugfixes:
		if (gt == BWAPI::UpgradeTypes::Adrenal_Glands)
			op->needs(1, unitMap[BWAPI::UnitTypes::Zerg_Hive]);
		
		BWAPI::UnitType ut = gt.whatsRequired(k);
		if (ut != BWAPI::UnitTypes::None)
			op->needs(1, unitMap[ut]);
		if (gt.mineralPrice(k) > 0)
			op->consums(gt.mineralPrice(k), minerals);
		if (gt.gasPrice(k) > 0)
			op->consums(gt.gasPrice(k), gas);
		op->locks(1, unitMap[what]);
		if (k > 1)
			op->needs(k-1, res);
		op->checkpoint("CUpgradeStart", gt.upgradeTime(k));
		op->unlocks(1, unitMap[what]);
		op->prods(1, res);
		op->checkpoint("CUpgradeFinished", 1);
		
		res->associated.push_back(op);
	}
}

void init()
{
	workerMap[BWAPI::Races::Terran]  = new ResourceDescription("TerranWorker", true);
	workerMap[BWAPI::Races::Protoss] = new ResourceDescription("ProtossWorker", true);
	workerMap[BWAPI::Races::Zerg]    = new ResourceDescription("ZergWorker", true);
	
	gasworkerMap[BWAPI::Races::Terran]  = new ResourceDescription("TerranGasWorker");
	gasworkerMap[BWAPI::Races::Protoss] = new ResourceDescription("ProtossGasWorker");
	gasworkerMap[BWAPI::Races::Zerg]    = new ResourceDescription("ZergGasWorker");
	
	supplyMap[BWAPI::Races::Terran]  = new ResourceDescription("TerranSupply", true);
	supplyMap[BWAPI::Races::Protoss] = new ResourceDescription("ProtossSupply", true);
	supplyMap[BWAPI::Races::Zerg]    = new ResourceDescription("ZergSupply", true);
	
	workingplaces = new ResourceDescription("GasWorkingPlaces", true);
	larva = new ResourceDescription("Larva", false, 240);

	minerals = new ResourceDescription("Minerals", false, 1000);
	minerals->addGrowth(45, workerMap[BWAPI::Races::Terran]);
	minerals->addGrowth(45, workerMap[BWAPI::Races::Protoss]);
	minerals->addGrowth(45, workerMap[BWAPI::Races::Zerg]);
	
	gas      = new ResourceDescription("Gas", false, 1000);
	gas->addGrowth(45, gasworkerMap[BWAPI::Races::Terran]);
	gas->addGrowth(45, gasworkerMap[BWAPI::Races::Protoss]);
	gas->addGrowth(45, gasworkerMap[BWAPI::Races::Zerg]);
	
	OperationDescription* op = new OperationDescription("SendTerranGasWorker");
	op->race = BWAPI::Races::Terran;
	op->locks(1, workerMap[BWAPI::Races::Terran]);
	op->locks(1, workingplaces);
	op->prods(1, gasworkerMap[BWAPI::Races::Terran]);
	op->checkpoint("CSendGasWorker", 1);
	gas->associated.push_back(op);
	
	op = new OperationDescription("ReturnTerranGasWorker");
	op->race = BWAPI::Races::Terran;
	op->consums(1, gasworkerMap[BWAPI::Races::Terran]);
	op->unlocks(1, workerMap[BWAPI::Races::Terran]);
	op->unlocks(1, workingplaces);
	op->checkpoint("CReturnGasWorker", 1);
	
	op = new OperationDescription("SendProtossGasWorker");
	op->race = BWAPI::Races::Protoss;
	op->locks(1, workerMap[BWAPI::Races::Protoss]);
	op->locks(1, workingplaces);
	op->prods(1, gasworkerMap[BWAPI::Races::Protoss]);
	op->checkpoint("CSendGasWorker", 1);
	gas->associated.push_back(op);
	
	op = new OperationDescription("ReturnProtossGasWorker");
	op->race = BWAPI::Races::Protoss;
	op->consums(1, gasworkerMap[BWAPI::Races::Protoss]);
	op->unlocks(1, workerMap[BWAPI::Races::Protoss]);
	op->unlocks(1, workingplaces);
	op->checkpoint("CReturnGasWorker", 1);
	
	op = new OperationDescription("SendZergGasWorker");
	op->race = BWAPI::Races::Zerg;
	op->locks(1, workerMap[BWAPI::Races::Zerg]);
	op->locks(1, workingplaces);
	op->prods(1, gasworkerMap[BWAPI::Races::Zerg]);
	op->checkpoint("CSendGasWorker", 1);
	gas->associated.push_back(op);
	
	op = new OperationDescription("ReturnZergGasWorker");
	op->race = BWAPI::Races::Zerg;
	op->consums(1, gasworkerMap[BWAPI::Races::Zerg]);
	op->unlocks(1, workerMap[BWAPI::Races::Zerg]);
	op->unlocks(1, workingplaces);
	op->checkpoint("CReturnGasWorker", 1);
	
	for (auto it : BWAPI::UnitTypes::allUnitTypes())
		writeResForUnitType(it);
	for (auto it : BWAPI::TechTypes::allTechTypes())
		writeResForTechType(it);
	for (auto it : BWAPI::UnitTypes::allUnitTypes())
		writeOpForUnitType(it);
	for (auto it : BWAPI::TechTypes::allTechTypes())
		writeOpForTechType(it);
	for (auto it : BWAPI::UpgradeTypes::allUpgradeTypes())
		writeOpForUpgradeType(it);
		
	larva->addGrowth(1, unitMap[BWAPI::UnitTypes::Zerg_Hatchery]);
	larva->addGrowth(1, unitMap[BWAPI::UnitTypes::Zerg_Lair]);
	larva->addGrowth(1, unitMap[BWAPI::UnitTypes::Zerg_Hive]);
}

void writeBWPlan()
{
	std::cout << "#include \"bwplan-internal.h\"\n\n";
	std::cout << "typedef ResourceIndex RI;\n";
	std::cout << "typedef OperationIndex OI;\n\n";
	std::cout << "std::string ResourceIndex::getName() const\n{\n"
			<< "\tswitch(index_)\n\t{\n";
	for (auto it : resourceDescriptions)
		std::cout << "\t\tcase " << it->name << ":\n\t\t\treturn \"" << it->name << "\";\n";
	std::cout << "\t\tdefault:\n"
				<< "\t\t\treturn \"[UNKNOWN]\";\n";
	std::cout << "\t}\n}\n\n";
	
	/*
	std::cout << "bool ResourceIndex::isLockable() const\n{\n"
			<< "\tswitch(index_)\n\t{\n";
	for (auto it : resourceDescriptions)
		if (!it->lockable)
			std::cout << "\t\tcase " << it->name << ":\n";
	std::cout << "\t\t\treturn false;\n";
	std::cout << "\t\tdefault:\n" << "\t\t\treturn true;\n";
	std::cout << "\t}\n}\n\n";
	*/
	
	std::cout << "bool ResourceIndex::isGrowthing() const\n{\n"
			<< "\tswitch(index_)\n\t{\n";
	for (auto it : resourceDescriptions)
		if (!it->growth.empty())
			std::cout << "\t\tcase " << it->name << ":\n";
	std::cout << "\t\t\treturn true;\n";
	std::cout << "\t\tdefault:\n" << "\t\t\treturn false;\n";
	std::cout << "\t}\n}\n\n";
	
	std::cout << "int ResourceIndex::getScaling() const\n{\n"
			<< "\tswitch(index_)\n\t{\n";
	for (auto it : resourceDescriptions)
		if (it->scaling != 1)
			std::cout << "\t\tcase " << it->name << ":\n"
					<< "\t\t\treturn " << it->scaling << ";\n";
	std::cout << "\t\tdefault:\n" << "\t\t\treturn 1;\n";
	std::cout << "\t}\n}\n\n";
	
	std::cout << "std::set<OperationIndex> ResourceIndex::getAssociatedOperations() const\n{\n"
			<< "\tstd::set<OperationIndex> result;\n"
			<< "\tswitch(index_)\n\t{\n";
	for (auto it : resourceDescriptions)
		if (!it->associated.empty()) {
			std::cout << "\t\tcase " << it->name << ":\n";
			for (auto ait : it->associated)
				std::cout << "\t\t\tresult.insert(OI::" << ait->name << ");\n";
			std::cout << "\t\t\tbreak;\n";
		}
	std::cout << "\t\tdefault:\n" << "\t\t\tbreak;\n";
	std::cout << "\t}\n\treturn result;\n}\n\n";
	
	std::cout << "BWAPI::Race ResourceIndex::associatedRace() const\n{\n"
			<< "\tswitch(index_)\n\t{\n";
	for (auto it : resourceDescriptions)
		if (it->race != BWAPI::Races::None)
			std::cout << "\t\tcase " << it->name << ":\n\t\t\treturn BWAPI::Races::" << it->race.getName() << ";\n";
	std::cout << "\t\tdefault:\n"
				<< "\t\t\treturn BWAPI::Races::None;\n";
	std::cout << "\t}\n}\n\n";
	
	std::cout << "BWAPI::UnitType ResourceIndex::associatedUnitType() const\n{\n"
			<< "\tswitch(index_)\n\t{\n";
	for (auto it : resourceDescriptions)
		if (it->ut != BWAPI::UnitTypes::None)
			std::cout << "\t\tcase " << it->name << ":\n\t\t\treturn BWAPI::UnitTypes::" << toCName(it->ut.getName()) << ";\n";
	std::cout << "\t\tdefault:\n"
				<< "\t\t\treturn BWAPI::UnitTypes::None;\n";
	std::cout << "\t}\n}\n\n";
	
	std::cout << "BWAPI::TechType ResourceIndex::associatedTechType() const\n{\n"
			<< "\tswitch(index_)\n\t{\n";
	for (auto it : resourceDescriptions)
		if (it->tt != BWAPI::TechTypes::None)
			std::cout << "\t\tcase " << it->name << ":\n\t\t\treturn BWAPI::TechTypes::" << toCName(it->tt.getName()) << ";\n";
	std::cout << "\t\tdefault:\n"
				<< "\t\t\treturn BWAPI::TechTypes::None;\n";
	std::cout << "\t}\n}\n\n";
	
	std::cout << "BWAPI::UpgradeType ResourceIndex::associatedUpgradeType() const\n{\n"
			<< "\tswitch(index_)\n\t{\n";
	for (auto it : resourceDescriptions)
		if (it->gt != BWAPI::UpgradeTypes::None)
			std::cout << "\t\tcase " << it->name << ":\n\t\t\treturn BWAPI::UpgradeTypes::" << toCName(it->gt.getName()) << ";\n";
	std::cout << "\t\tdefault:\n"
				<< "\t\t\treturn BWAPI::UpgradeTypes::None;\n";
	std::cout << "\t}\n}\n\n";
	
	std::cout << "int Resources::getGrowth(const ResourceIndex& ri) const\n{\n"
			<< "\tswitch(ri.getType())\n\t{\n";
	for (auto it : resourceDescriptions)
		if (!it->growth.empty()) {
			std::cout << "\t\tcase RI::" << it->name << ":\n"
					<< "\t\t\treturn ";
			for (auto git : it->growth) {
				if (git != *it->growth.begin()) std::cout << " + ";
				std::cout << git.first << " * amount[RI::" << git.second->name << "]";
			}
			std::cout << ";\n";
		}
	std::cout << "\t\tdefault:\n" << "\t\t\treturn 0;\n";
	std::cout << "\t}\n}\n\n";
	
	std::cout << "void Resources::advance(int dt)\n{\n";
	for (auto it : resourceDescriptions)
		if (!it->growth.empty())
			std::cout << "\tamount[RI::" << it->name << "] += dt * getGrowth(RI::" << it->name << ");\n";
	std::cout << "\ttime += dt;\n";
	std::cout << "}\n\n";
	
	std::cout << "void Resources::inc(const ResourceIndex& ri, int optime, int value)\n{\n"
				<< "\tswitch(ri.getType())\n\t{\n";
	for (auto it : resourceDescriptions)
		if (!it->influence.empty()) {
			std::cout << "\t\tcase RI::" << it->name << ":\n";
			for (auto iit : it->influence)
				std::cout << "\t\t\tamount[RI::" << iit.second->name << "] -= " << iit.first << " * (optime - time) * value;\n";
			std::cout << "\t\t\tbreak;\n";
		}
	std::cout << "\t\tdefault:\n"
				<< "\t\t\tbreak;\n";
	std::cout << "\t}\n";
	std::cout << "\tamount[ri.getIndex()] += value * ri.getScaling();\n";
	std::cout << "}\n\n";
	
	std::cout << "std::string OperationIndex::getName() const\n{\n"
			<< "\tswitch(index_)\n\t{\n";
	for (auto it : operationDescriptions)
		std::cout << "\t\tcase " << it->name << ":\n\t\t\treturn \"" << it->name << "\";\n";
	std::cout << "\t\tdefault:\n"
				<< "\t\t\treturn \"[UNKNOWN]\";\n";
	std::cout << "\t}\n}\n\n";
	
	std::cout << "BWAPI::Race OperationIndex::associatedRace() const\n{\n"
			<< "\tswitch(index_)\n\t{\n";
	for (auto it : operationDescriptions)
		if (it->race != BWAPI::Races::None)
			std::cout << "\t\tcase " << it->name << ":\n\t\t\treturn BWAPI::Races::" << it->race.getName() << ";\n";
	std::cout << "\t\tdefault:\n"
				<< "\t\t\treturn BWAPI::Races::None;\n";
	std::cout << "\t}\n}\n\n";
	
	std::cout << "BWAPI::UnitType OperationIndex::associatedUnitType() const\n{\n"
			<< "\tswitch(index_)\n\t{\n";
	for (auto it : operationDescriptions)
		if (it->ut != BWAPI::UnitTypes::None)
			std::cout << "\t\tcase " << it->name << ":\n\t\t\treturn BWAPI::UnitTypes::" << toCName(it->ut.getName()) << ";\n";
	std::cout << "\t\tdefault:\n"
				<< "\t\t\treturn BWAPI::UnitTypes::None;\n";
	std::cout << "\t}\n}\n\n";
	
	std::cout << "BWAPI::TechType OperationIndex::associatedTechType() const\n{\n"
			<< "\tswitch(index_)\n\t{\n";
	for (auto it : operationDescriptions)
		if (it->tt != BWAPI::TechTypes::None)
			std::cout << "\t\tcase " << it->name << ":\n\t\t\treturn BWAPI::TechTypes::" << toCName(it->tt.getName()) << ";\n";
	std::cout << "\t\tdefault:\n"
				<< "\t\t\treturn BWAPI::TechTypes::None;\n";
	std::cout << "\t}\n}\n\n";
	
	std::cout << "BWAPI::UpgradeType OperationIndex::associatedUpgradeType() const\n{\n"
			<< "\tswitch(index_)\n\t{\n";
	for (auto it : operationDescriptions)
		if (it->gt != BWAPI::UpgradeTypes::None)
			std::cout << "\t\tcase " << it->name << ":\n\t\t\treturn BWAPI::UpgradeTypes::" << toCName(it->gt.getName()) << ";\n";
	std::cout << "\t\tdefault:\n"
				<< "\t\t\treturn BWAPI::UpgradeTypes::None;\n";
	std::cout << "\t}\n}\n\n";
	
	for (auto it : operationDescriptions)
		it->calculate();
	
	std::cout << "TimeType Operation::duration() const\n{\n"
			<< "\tswitch(index_.getType())\n\t{\n";
	for (auto it : operationDescriptions)
		std::cout << "\t\tcase OI::" << it->name << ":\n\t\t\treturn " << it->duration << ";\n";
	std::cout << "\t\tdefault:\n"
				<< "\t\t\treturn 0;\n";
	std::cout << "\t}\n}\n\n";
	
	std::cout << "int Operation::stageCount() const\n{\n"
			<< "\tswitch(index_.getType())\n\t{\n";
	for (auto it : operationDescriptions)
		std::cout << "\t\tcase OI::" << it->name << ":\n\t\t\treturn " << it->stagecount << ";\n";
	std::cout << "\t\tdefault:\n"
				<< "\t\t\treturn 0;\n";
	std::cout << "\t}\n}\n\n";
	
	std::cout << "TimeType Operation::stageDuration(int stage) const\n{\n"
			<< "\tswitch(index_.getType())\n\t{\n";
	for (auto it : operationDescriptions) {
		std::cout << "\t\tcase OI::" << it->name << ":\n";
		std::cout << "\t\t\tswitch(stage)\n\t\t\t{\n";
		int counter = 0;
		for (auto iit : it->items)
			if (iit.type == ItemDescription::CheckPoint)
		{
			std::cout << "\t\t\t\tcase " << counter << ":\n"
					<< "\t\t\t\t\treturn " << iit.count << ";\n";
			++counter;
		}
		std::cout << "\t\t\t\tdefault:\n"
					<< "\t\t\t\t\treturn 0;\n"
					<< "\t\t\t}\n";
	}
	std::cout << "\t\tdefault:\n"
				<< "\t\t\treturn 0;\n";
	std::cout << "\t}\n}\n\n";
	
	std::cout << "CheckPointResult::type Operation::executeInternal()\n{\n"
			<< "\tswitch(index_.getType())\n\t{\n";
	for (auto it : operationDescriptions) {
		std::cout << "\t\tcase OI::" << it->name << ":\n";
		std::cout << "\t\t\tswitch(stage_)\n\t\t\t{\n";
		int counter = 0;
		for (auto iit : it->items)
			if (iit.type == ItemDescription::CheckPoint)
		{
			std::cout << "\t\t\t\tcase " << counter << ":\n"
					<< "\t\t\t\t\treturn " << iit.name << "(*this);\n";
			++counter;
		}
		std::cout << "\t\t\t}\n\t\t\treturn CheckPointResult::failed;\n";
	}
	std::cout << "\t\tdefault:\n\t\t\treturn CheckPointResult::failed;\n";
	std::cout << "\t}\n}\n\n";
	
	std::cout << "TimeType Operation::firstApplyableAt(const Resources& res, int stage, ResourceIndex& blocking) const\n{\n"
			<< "\tTimeType result = 0;\n"
			<< "\tswitch(index_.getType())\n\t{\n";
	for (auto it : operationDescriptions) {
		std::cout << "\t\tcase OI::" << it->name << ":\n";
		std::cout << "\t\t\tswitch(stage)\n\t\t\t{\n";
		std::cout << "\t\t\t\tcase 0:\n";
		int counter = 0;
		for (auto iit : it->items) {
			switch (iit.type)
			{
				case ItemDescription::CheckPoint:
					++counter;
					std::cout << "\t\t\t\t\treturn result;\n"
							<< "\t\t\t\tcase " << counter << ":\n";
					break;
				case ItemDescription::Needs:
					std::cout << "\t\t\t\t\tNeeds(res, " << iit.count << ", RI::" << iit.res->name << ", result, blocking);\n";
					break;
				case ItemDescription::Locks:
					std::cout << "\t\t\t\t\tLocks(res, " << iit.count << ", RI::" << iit.res->name << ", result, blocking);\n";
					break;
				case ItemDescription::Consums:
					std::cout << "\t\t\t\t\tConsums(res, " << iit.count << ", RI::" << iit.res->name << ", result, blocking);\n";
					break;
				default:
					break;
			}
		}
		std::cout << "\t\t\t\t\treturn result;\n";
		std::cout << "\t\t\t}\n";
	}
	std::cout << "\t\tdefault:\n\t\t\tbreak;\n";
	std::cout << "\t}\n\treturn result;\n}\n\n";
	
	std::cout << "void Operation::apply(Resources& res, const TimeInterval& interval, bool pushdecs) const\n{\n"
			<< "\tTimeType applytime = scheduledtime_;\n"
			<< "\tswitch(index_.getType())\n\t{\n";
	for (auto it : operationDescriptions) {
		std::cout << "\t\tcase OI::" << it->name << ":\n"
				<< "\t\t\tswitch(stage_)\n\t\t\t{\n"
				<< "\t\t\t\tcase 0:\n";
		int counter = 0;
		for (auto iit : it->items) {
			switch (iit.type)
			{
				case ItemDescription::CheckPoint:
					++counter;
					std::cout << "\t\t\t\t\tapplytime += " << iit.count << ";\n"
							<< "\t\t\t\tcase " << counter << ":\n";
					break;
				/*case ItemDescription::Needs:
					std::cout << "\t\t\t\t\tNeeds(res, " << iit.count << ", RI::" << iit.res->name << ", interval, applytime, pushdecs);\n";
					break;*/
				case ItemDescription::Locks:
					std::cout << "\t\t\t\t\tLocks(res, " << iit.count << ", RI::" << iit.res->name << ", interval, applytime, pushdecs);\n";
					break;
				case ItemDescription::Unlocks:
					std::cout << "\t\t\t\t\tUnlocks(res, " << iit.count << ", RI::" << iit.res->name << ", interval, applytime, pushdecs);\n";
					break;
				case ItemDescription::Consums:
					std::cout << "\t\t\t\t\tConsums(res, " << iit.count << ", RI::" << iit.res->name << ", interval, applytime, pushdecs);\n";
					break;
				case ItemDescription::Prods:
					std::cout << "\t\t\t\t\tProds(res, " << iit.count << ", RI::" << iit.res->name << ", interval, applytime, pushdecs);\n";
					break;
				default:
					break;
			}
		}
		std::cout << "\t\t\t\tdefault:\n\t\t\t\t\tbreak;\n"
				<< "\t\t\t}\n";
		std::cout << "\t\t\tbreak;\n";
	}
	std::cout << "\t\tdefault:\n\t\t\tbreak;\n";
	std::cout << "\t}\n}\n\n";
}

struct CompareByLockable
{
	bool operator () (ResourceDescription* a, ResourceDescription* b) const
	{
		if (a->lockable != b->lockable)
			return a->lockable;
			
		if (a->isTech() != b->isTech())
			return a->isTech();
		
		if (a->isUpgrade() != b->isUpgrade())
			return a->isUpgrade();
	
		return a->name < b->name;
	}
};

int main(int argc, char *argv[])
{
	if (argc < 2) {
		std::cerr << "Use: BWPlanWriter [filename]\n";
		return 1;
	}
	
	BWAPI::BWAPI_init();
	init();

	std::string filename(argv[1]);
	if (filename == "resourceenum.h") {
		std::sort(resourceDescriptions.begin(), resourceDescriptions.end(), CompareByLockable());
		
		std::stringstream stream;
		
		auto it = resourceDescriptions.begin();
		while ((*it)->lockable) ++it;
		stream << ",\n\t\tIndexLockedEnd = " << (*it)->name;
		stream << ",\n\t\tIndexTechBegin = " << (*it)->name;
		while ((*it)->isTech()) ++it;
		stream << ",\n\t\tIndexTechEnd = " << (*it)->name;
		stream << ",\n\t\tIndexUpgradeBegin = " << (*it)->name;
		while ((*it)->isUpgrade()) ++it;
		stream << ",\n\t\tIndexUpgradeEnd = " << (*it)->name;
		
		writeStringList(resourceDescriptions, stream.str());
	} else if (filename == "operationenum.h") {
		writeStringList(operationDescriptions);
	} else if (filename == "bwplan.cpp") {
		writeHeader();
		writeBWPlan();
	} else {
		std::cerr << "Unknown filename.\n";
		return 1;
	}
	return 0;
}
