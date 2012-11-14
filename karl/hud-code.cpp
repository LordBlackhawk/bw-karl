// ToDo:
//  * LinePrinter class + onPrintLine event to put infos.

#include "hud-code.hpp"
#include "timer.hpp"
#include "code-list.hpp"
#include <BWTA.h>
#include <cstdarg>
#include <cstdlib>
#include <cstdio>

using namespace BWAPI;

namespace
{
    void drawTerrainData()
    {
        //we will iterate through all the base locations, and draw their outlines.
        for (auto i : BWTA::getBaseLocations()) {
            TilePosition p = i->getTilePosition();
            Position c     = i->getPosition();

            //draw outline of center location
            Broodwar->drawBox(CoordinateType::Map,p.x()*32,p.y()*32,p.x()*32+4*32,p.y()*32+3*32,Colors::Blue,false);

            //draw a circle at each mineral patch
            for (auto j : i->getStaticMinerals()) {
                Position q = j->getInitialPosition();
                Broodwar->drawCircle(CoordinateType::Map,q.x(),q.y(),30,Colors::Cyan,false);
            }

            //draw the outlines of vespene geysers
            for (auto j : i->getGeysers()) {
                TilePosition q = j->getInitialTilePosition();
                Broodwar->drawBox(CoordinateType::Map,q.x()*32,q.y()*32,q.x()*32+4*32,q.y()*32+2*32,Colors::Orange,false);
            }

            //if this is an island expansion, draw a yellow circle around the base location
            if (i->isIsland())
                Broodwar->drawCircle(CoordinateType::Map,c.x(),c.y(),80,Colors::Yellow,false);
        }

        //we will iterate through all the regions and draw the polygon outline of it in green.
        for (auto r : BWTA::getRegions()) {
            BWTA::Polygon p = r->getPolygon();
            for(int j=0; j<(int)p.size(); ++j) {
                Position point1 = p[j];
                Position point2 = p[(j+1) % p.size()];
                Broodwar->drawLine(CoordinateType::Map,point1.x(),point1.y(),point2.x(),point2.y(),Colors::Green);
            }
        }

        //we will visualize the chokepoints with red lines
        for (auto c : BWTA::getChokepoints()) {
            Position point1 = c->getSides().first;
            Position point2 = c->getSides().second;
            Broodwar->drawLine(CoordinateType::Map,point1.x(),point1.y(),point2.x(),point2.y(),Colors::Red);
        }
    }
    
    void drawBullets()
    {
        Player* self = Broodwar->self();
        for (auto i : Broodwar->getBullets()) {
            Position p = i->getPosition();
            double velocityX = i->getVelocityX();
            double velocityY = i->getVelocityY();
            if (i->getPlayer() == self) {
              Broodwar->drawLineMap(p.x(),p.y(),p.x()+(int)velocityX,p.y()+(int)velocityY,Colors::Green);
              Broodwar->drawTextMap(p.x(),p.y(),"\x07%s", i->getType().getName().c_str());
            } else {
              Broodwar->drawLineMap(p.x(),p.y(),p.x()+(int)velocityX,p.y()+(int)velocityY,Colors::Red);
              Broodwar->drawTextMap(p.x(),p.y(),"\x06%s", i->getType().getName().c_str());
            }
        }
    }
    
    void drawStats(HUDTextOutput& hud)
    {
        hud.printf("I have %d units:", Broodwar->self()->getUnits().size());
        std::map<UnitType, int> unitTypeCounts;
        for (auto i : Broodwar->self()->getUnits()) {
            if (unitTypeCounts.find(i->getType())==unitTypeCounts.end()) {
                unitTypeCounts.insert(std::make_pair(i->getType(),0));
            }
            unitTypeCounts.find(i->getType())->second++;
        }
     
        for (auto i : unitTypeCounts)
            hud.printf("- %d %ss", i.second, i.first.getName().c_str());
    }

    bool showterrain = false;
    bool showbullets = false;
    bool showstats   = false;
    bool showfps     = true;
    bool showtiming  = true;
    bool showplan    = true;
}

void HUDCode::onTick()
{
    HUDTextOutput hud;

    if (showtiming) {
        hud.printf("Current frame: %d", Broodwar->getFrameCount());
        hud.printf("Reaction time: %5.0f / %5.0f / %5.0f ns", timerMin(), timerAverage(), timerMax());
    }
    if (showterrain)
        drawTerrainData();
    if (showbullets)
        drawBullets();
    if (showstats)
        drawStats(hud);
    if (showfps)
        Broodwar->drawTextScreen(300, 0, "FPS: %f", Broodwar->getAverageFPS());
    if (showplan)
        CodeList::onDrawPlan(hud);
}

void HUDCode::onPausedTick()
{
    onTick();
}

void HUDCode::onSendText(const std::string& text)
{
    if (text == "/show terrain")
        showterrain = !showterrain;
    if (text == "/show bullets")
        showbullets = !showbullets;
    if (text == "/show stats")
        showstats = !showstats;
    if (text == "/show fps")
        showfps = !showfps;
    if (text == "/show timing")
        showtiming = !showtiming;
    if (text == "/show plan")
        showplan = !showplan;
}

void HUDTextOutput::printf(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int buffsize = _vscprintf(fmt, ap);
    char* buffer = (char*)malloc(buffsize+1);
    vsprintf(buffer, fmt, ap);
    BWAPI::Broodwar->drawTextScreen(5, 2+16*line, "%s", buffer);
    free(buffer);
    va_end(ap);
    ++line;
}
