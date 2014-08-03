#include "broodwar-scanners.hpp"
#include "broodwar-events.hpp"

CreepScannerAction::CreepScannerAction()
    : AbstractAction(NULL)
{ }

void CreepScannerAction::onBegin(AbstractExecutionEngine* /*engine*/)
{
    int mapWidth  = BWAPI::Broodwar->mapWidth();
    int mapHeight = BWAPI::Broodwar->mapHeight();
    fields.resize(mapWidth, mapHeight);
    for (int x=0; x<mapWidth; ++x)
        for (int y=0; y<mapHeight; ++y)
            fields[x][y] = false;
}

CreepScannerAction::Status CreepScannerAction::onTick(AbstractExecutionEngine* engine)
{
    const int blocks = 4;
    int blockSizeX = BWAPI::Broodwar->mapWidth() / blocks;
    int blockSizeY = BWAPI::Broodwar->mapHeight() / blocks;
    int frame = BWAPI::Broodwar->getFrameCount();
    int nx = frame % blocks;
    int ny = (frame / blocks) % blocks;
    int maxX = (1+nx) * blockSizeX;
    int maxY = (1+ny) * blockSizeY;
    for (int x=maxX-blockSizeX; x<maxX; ++x)
        for (int y=maxY-blockSizeY; y<maxY; ++y)
            if (BWAPI::Broodwar->isVisible(x, y))
    {
        auto field = fields[x][y];
        bool creep = BWAPI::Broodwar->hasCreep(x, y);
        if (field != creep) {
            field = creep;
            engine->generateEvent(new CreepChangedEvent(BWAPI::TilePosition(x, y), creep));
        }
    }
    return Running;
}
