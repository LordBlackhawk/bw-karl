#include "blackboard-informations.hpp"
#include "broodwar-boundary-items.hpp"
#include <iostream>

BaseLocation::BaseLocation()
    : origin(NULL)
{ }

BlackboardInformations::~BlackboardInformations()
{
    for (auto it : allBaseLocations)
        delete it;
}

void BlackboardInformations::prepare()
{
    self = BWAPI::Broodwar->self();

    int mapWidth  = BWAPI::Broodwar->mapWidth();
    int mapHeight = BWAPI::Broodwar->mapHeight();
    fields.resize(mapWidth, mapHeight);
    for (int x=0; x<mapWidth; ++x)
        for (int y=0; y<mapHeight; ++y)
    {
        FieldInformations& field = fields[x][y];
        field.buildable = BWAPI::Broodwar->isBuildable(x, y);
        field.creep     = BWAPI::Broodwar->hasCreep(x, y);
    }

    auto mybase = BWTA::getStartLocation(self);
    for (auto base : BWTA::getBaseLocations()) {
        auto baselocation = new BaseLocation;
        baselocation->origin = base;
        for (auto unit : base->getMinerals())
            baselocation->minerals.insert(new ResourceBoundaryItem(unit, unit->getType(), &fields, baselocation));
        allBaseLocations.insert(baselocation);
        if (base == mybase)
            ownBaseLocations.insert(baselocation);
    }
}

void BlackboardInformations::printFieldInformations(std::ostream& stream)
{
    int mapWidth  = fields.getWidth();
    int mapHeight = fields.getHeight();
    stream << "\n";
    for (int y=0; y<mapHeight; ++y) {
        for (int x=0; x<mapWidth; ++x) {
            FieldInformations& field = fields[x][y];
            char c = '.';
            if (!field.buildable) {
                c = '#';
            } else if (field.blocker != NULL) {
                c = 'x';
            } else if (field.creep) {
                c = '~';
            }
            stream << c;
        }
        stream << "\n";
    }
}
