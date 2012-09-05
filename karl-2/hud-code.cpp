// ToDo:
//  * LinePrinter class + onPrintLine event to put infos.

#include "hud-code.hpp"
#include "timer.hpp"
#include <BWTA.h>

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
	
	void drawStats()
	{
		Broodwar->drawTextScreen(5,16,"I have %d units:", Broodwar->self()->getUnits().size());
		std::map<UnitType, int> unitTypeCounts;
		for (auto i : Broodwar->self()->getUnits()) {
			if (unitTypeCounts.find(i->getType())==unitTypeCounts.end()) {
				unitTypeCounts.insert(std::make_pair(i->getType(),0));
			}
			unitTypeCounts.find(i->getType())->second++;
		}
	 
		int line=2;
		for (auto i : unitTypeCounts) {
			Broodwar->drawTextScreen(5,16*line,"- %d %ss", i.second, i.first.getName().c_str());
			++line;
		}
	}

	bool showterrain = false;
	bool showbullets = false;
	bool showstats   = false;
	bool showfps     = true;
	bool showtiming  = true;
}

void HUDCode::onTick()
{
	if (showterrain)
		drawTerrainData();
	if (showbullets)
		drawBullets();
	if (showstats)
		drawStats();
	if (showfps)
		Broodwar->drawTextScreen(300, 0, "FPS: %f", Broodwar->getAverageFPS());
	if (showtiming)
		Broodwar->drawTextScreen(5, 2, "Reaction time: %.0f / %.0f / %.0f ns", timerMin(), timerAverage(), timerMax());
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
}
