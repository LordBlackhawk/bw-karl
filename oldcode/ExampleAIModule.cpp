#include "ExampleAIModule.h"
#include "interface.h"
#include "threatmap.h"
#include "logfile.h"

void ExampleAIModule::onStart()
{
	inter = NULL;
	LogFile::write("onStart()");

	//Broodwar->printf("The map is %s, a %d player map", Broodwar->mapName().c_str(),
	//	(int)Broodwar->getStartLocations().size());
	// Enable some cheat flags
	Broodwar->enableFlag(Flag::UserInput);
	// Uncomment to enable complete map information
	//Broodwar->enableFlag(Flag::CompleteMapInformation);

	show_terrain			= false;
	show_plan				= false;
	show_bullets			= false;
	show_visibility_data	= false;
	show_threat				= false;

	/*
	if (Broodwar->isReplay())
	{
		Broodwar->printf("The following players are in this replay:");
		for(std::set<Player*>::iterator p=Broodwar->getPlayers().begin();p!=Broodwar->getPlayers().end();p++)
		{
			if (!(*p)->getUnits().empty() && !(*p)->isNeutral())
			{
				Broodwar->printf("%s, playing as a %s",(*p)->getName().c_str(),(*p)->getRace().getName().c_str());
			}
		}
	}
	*/

	if (!Broodwar->isReplay()) {
		Broodwar->printf("The match up is %s v %s",
			Broodwar->self()->getRace().getName().c_str(),
			Broodwar->enemy()->getRace().getName().c_str());

		//read map information into BWTA so terrain analysis can be done in another threat
		BWTA::readMap();
		BWTA::analyze();
		inter = new Interface();
	}
}

void ExampleAIModule::onEnd(bool isWinner)
{
	if (inter != NULL) {
		delete inter;
		inter = NULL;
	}
}

void ExampleAIModule::onFrame()
{
	if (show_visibility_data)
		drawVisibilityData();

	if (show_bullets)
		drawBullets();

	if (Broodwar->isReplay())
		return;

	drawStats();

	if ((inter != NULL) && (show_plan))
		inter->drawTaskInfo();

	if (show_terrain)
		drawTerrainData();

	if (show_threat)
		drawThreatData();

	if (inter != NULL)
		inter->onFrame();
}

void ExampleAIModule::onSendText(std::string text)
{
	if (text == "/slow") {
		Broodwar->setLocalSpeed(-1);
	} else if (text == "/fast") {
		Broodwar->setLocalSpeed(5);
	} else if (text == "/show terrain") {
		show_terrain = !show_terrain;
	} else if (text == "/show plan") {
		show_plan = !show_plan;
	} else if (text == "/show threat") {
		show_threat = !show_threat;
	} else if (text == "/show bullets") {
		show_bullets = !show_bullets;
	} else if (text == "/show players") {
		showPlayers();
	} else if (text == "/show forces") {
		showForces();
	} else if (text == "/show visibility") {
		show_visibility_data=!show_visibility_data;
	} else {
		Broodwar->printf("You typed '%s'!",text.c_str());
		Broodwar->sendText("%s",text.c_str());
	}
}

void ExampleAIModule::onReceiveText(BWAPI::Player* player, std::string text)
{
  Broodwar->printf("%s said '%s'", player->getName().c_str(), text.c_str());
}

void ExampleAIModule::onPlayerLeft(BWAPI::Player* player)
{
  Broodwar->sendText("%s left the game.",player->getName().c_str());
}

void ExampleAIModule::onNukeDetect(BWAPI::Position target)
{
  if (target!=Positions::Unknown)
    Broodwar->printf("Nuclear Launch Detected at (%d,%d)",target.x(),target.y());
  else
    Broodwar->printf("Nuclear Launch Detected");
}

void ExampleAIModule::onUnitDiscover(BWAPI::Unit* unit)
{
  if (inter != NULL)
		inter->onUnitDiscover(unit);
}

void ExampleAIModule::onUnitEvade(BWAPI::Unit* unit)
{
  if (inter != NULL)
		inter->onUnitEvade(unit);
}

void ExampleAIModule::onUnitShow(BWAPI::Unit* unit)
{
  if (inter != NULL)
		inter->onUnitShow(unit);
}

void ExampleAIModule::onUnitHide(BWAPI::Unit* unit)
{
  if (inter != NULL)
		inter->onUnitHide(unit);
}

void ExampleAIModule::onUnitCreate(BWAPI::Unit* unit)
{
	if (inter != NULL)
		inter->onUnitCreate(unit);
}

void ExampleAIModule::onUnitDestroy(BWAPI::Unit* unit)
{
	if (inter != NULL)
		inter->onUnitDestroy(unit);
}

void ExampleAIModule::onUnitMorph(BWAPI::Unit* unit)
{
	if (inter != NULL)
		inter->onUnitMorph(unit);
}

void ExampleAIModule::onUnitRenegade(BWAPI::Unit* unit)
{
	if (inter != NULL)
		inter->onUnitRenegade(unit);
}

void ExampleAIModule::onSaveGame(std::string gameName)
{
  Broodwar->printf("The game was saved to \"%s\".", gameName.c_str());
}

void ExampleAIModule::drawStats()
{
	int seconds=Broodwar->getFrameCount()/24;
	int minutes=seconds/60;
	seconds%=60;
	Broodwar->drawTextScreen(5, 16, "Game Time: %.2d:%.2d (%.5d)",
		minutes, seconds, Broodwar->getFrameCount());

	std::set<Unit*> myUnits = Broodwar->self()->getUnits();
	Broodwar->drawTextScreen(5,32,"I have %d units:", (int)myUnits.size());
	std::map<UnitType, int> unitTypeCounts;
	for(std::set<Unit*>::iterator i=myUnits.begin();i!=myUnits.end();++i)
	{
		if (!(*i)->getType().isBuilding()) {
			if (unitTypeCounts.find((*i)->getType())==unitTypeCounts.end())
			{
				unitTypeCounts.insert(std::make_pair((*i)->getType(),0));
			}
			unitTypeCounts.find((*i)->getType())->second++;
		}
	}
	int line=1;
	for(std::map<UnitType,int>::iterator i=unitTypeCounts.begin();i!=unitTypeCounts.end();++i)
	{
		Broodwar->drawTextScreen(5,16*(line+2),"- %d %ss",(*i).second, (*i).first.getName().c_str());
		line++;
  }
}

void ExampleAIModule::drawBullets()
{
	std::set<Bullet*> bullets = Broodwar->getBullets();
	for(std::set<Bullet*>::iterator i=bullets.begin();i!=bullets.end();++i)
	{
		Position p=(*i)->getPosition();
		double velocityX = (*i)->getVelocityX();
		double velocityY = (*i)->getVelocityY();
		if ((*i)->getPlayer()==Broodwar->self())
		{
			Broodwar->drawLineMap(p.x(),p.y(),p.x()+(int)velocityX,p.y()+(int)velocityY,Colors::Green);
			Broodwar->drawTextMap(p.x(),p.y(),"\x07%s",(*i)->getType().getName().c_str());
		}
		else
		{
			Broodwar->drawLineMap(p.x(),p.y(),p.x()+(int)velocityX,p.y()+(int)velocityY,Colors::Red);
			Broodwar->drawTextMap(p.x(),p.y(),"\x06%s",(*i)->getType().getName().c_str());
		}
	}
}

void ExampleAIModule::drawVisibilityData()
{
	for(int x=0;x<Broodwar->mapWidth();x++)
	{
		for(int y=0;y<Broodwar->mapHeight();y++)
		{
			if (Broodwar->isExplored(x,y))
			{
				if (Broodwar->isVisible(x,y))
					Broodwar->drawDotMap(x*32+16,y*32+16,Colors::Green);
				else
					Broodwar->drawDotMap(x*32+16,y*32+16,Colors::Blue);
			}
			else
				Broodwar->drawDotMap(x*32+16,y*32+16,Colors::Red);
		}
	}
}

void ExampleAIModule::drawThreatData()
{
	ThreatMap& tm = ThreatMap::getInstance();
	for(int x=0;x<Broodwar->mapWidth();x++)
	{
		for(int y=0;y<Broodwar->mapHeight();y++)
		{
			if (tm.isThreatPosition(TilePosition(x, y), true))
				Broodwar->drawDotMap(x*32+16, y*32+16, Colors::Red);
			else
				Broodwar->drawDotMap(x*32+16, y*32+16, Colors::Green);
		}
	}
}

void ExampleAIModule::drawTerrainData()
{
	//we will iterate through all the base locations, and draw their outlines.
	for(std::set<BWTA::BaseLocation*>::const_iterator i=BWTA::getBaseLocations().begin();i!=BWTA::getBaseLocations().end();++i)
	{
		TilePosition p=(*i)->getTilePosition();
		Position c=(*i)->getPosition();

		//draw outline of center location
		Broodwar->drawBox(CoordinateType::Map,p.x()*32,p.y()*32,p.x()*32+4*32,p.y()*32+3*32,Colors::Blue,false);

		//draw a circle at each mineral patch
		for(std::set<BWAPI::Unit*>::const_iterator j=(*i)->getStaticMinerals().begin();j!=(*i)->getStaticMinerals().end();++j)
		{
			Position q=(*j)->getInitialPosition();
			Broodwar->drawCircle(CoordinateType::Map,q.x(),q.y(),30,Colors::Cyan,false);
		}

		//draw the outlines of vespene geysers
		for(std::set<BWAPI::Unit*>::const_iterator j=(*i)->getGeysers().begin();j!=(*i)->getGeysers().end();++j)
		{
			TilePosition q=(*j)->getInitialTilePosition();
			Broodwar->drawBox(CoordinateType::Map,q.x()*32,q.y()*32,q.x()*32+4*32,q.y()*32+2*32,Colors::Orange,false);
		}

		//if this is an island expansion, draw a yellow circle around the base location
		if ((*i)->isIsland())
			Broodwar->drawCircle(CoordinateType::Map,c.x(),c.y(),80,Colors::Yellow,false);
	}

	//we will iterate through all the regions and draw the polygon outline of it in green.
	for(std::set<BWTA::Region*>::const_iterator r=BWTA::getRegions().begin();r!=BWTA::getRegions().end();++r)
	{
		typedef BWTA::Polygon::size_type size_type;
		BWTA::Polygon p=(*r)->getPolygon();
		for(size_type j=0; j<p.size(); j++)
		{
			Position point1=p[j];
			Position point2=p[(j+1) % p.size()];
			Broodwar->drawLine(CoordinateType::Map,point1.x(),point1.y(),point2.x(),point2.y(),Colors::Green);
		}
	}

	//we will visualize the chokepoints with red lines
	for(std::set<BWTA::Region*>::const_iterator r=BWTA::getRegions().begin();r!=BWTA::getRegions().end();++r)
	{
		for(std::set<BWTA::Chokepoint*>::const_iterator c=(*r)->getChokepoints().begin();c!=(*r)->getChokepoints().end();++c)
		{
			Position point1=(*c)->getSides().first;
			Position point2=(*c)->getSides().second;
			Broodwar->drawLine(CoordinateType::Map,point1.x(),point1.y(),point2.x(),point2.y(),Colors::Red);
		}
	}
}

void ExampleAIModule::showPlayers()
{
	std::set<Player*> players=Broodwar->getPlayers();
	for(std::set<Player*>::iterator i=players.begin();i!=players.end();++i)
	{
		Broodwar->printf("Player [%d]: %s is in force: %s",(*i)->getID(),(*i)->getName().c_str(), (*i)->getForce()->getName().c_str());
	}
}

void ExampleAIModule::showForces()
{
	std::set<Force*> forces=Broodwar->getForces();
	for(std::set<Force*>::iterator i=forces.begin();i!=forces.end();++i)
	{
		std::set<Player*> players=(*i)->getPlayers();
		Broodwar->printf("Force %s has the following players:",(*i)->getName().c_str());
		for(std::set<Player*>::iterator j=players.begin();j!=players.end();++j)
		{
			Broodwar->printf("  - Player [%d]: %s",(*j)->getID(),(*j)->getName().c_str());
		}
	}
}
