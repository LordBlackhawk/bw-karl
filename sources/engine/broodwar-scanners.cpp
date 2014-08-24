#include "broodwar-scanners.hpp"
#include "broodwar-events.hpp"

FieldScannerAction::FieldScannerAction()
    : AbstractAction(NULL)
{ }

FieldScannerAction::Status FieldScannerAction::onTick(AbstractExecutionEngine* engine)
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
        engine->generateEvent(new FieldSeenEvent(BWAPI::TilePosition(x, y), BWAPI::Broodwar->hasCreep(x, y)));
    }
    return Running;
}
