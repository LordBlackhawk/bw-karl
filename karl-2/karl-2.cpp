#include "utils/debug-broodwar.h"
#include "utils/debug-1.h"

#include "timer.hpp"
#include "code-list.hpp"
#include "strategie.hpp"

#include <BWAPI.h>
#include <BWAPI\Client.h>
#include <BWTA.h>

#include <windows.h>

using namespace BWAPI;

void reconnect()
{
	while(!BWAPIClient.connect())
	{
		Sleep(1000);
	}
}
int main(int /*argc*/, const char** /*argv[]*/)
{
	timerInit();
	BWAPI::BWAPI_init();
	LOG << "Connecting...";
	reconnect();
	while(true)
	{
		LOG << "Waiting to enter match...";
		while (!Broodwar->isInGame())
		{
			BWAPI::BWAPIClient.update();
			if (!BWAPI::BWAPIClient.isConnected())
			{
				LOG << "Reconnecting...";
				reconnect();
			}
		}

		LOG << "Reading terran information...";
		BWTA::readMap();
		BWTA::analyze();
		
		LOG << "Starting match...";
		Broodwar->sendText("Hello world from Karl 2.0!");
		Broodwar->enableFlag(Flag::UserInput);
		Broodwar->setLocalSpeed(0);
		
		LOG << "Calling onMatchBegin...";
		CodeList::onMatchBegin();
		
		LOG << "Loading strategie...";
		loadStrategie();
		
		LOG1 << "Latency Frames: " << BWAPI::Broodwar->getRemainingLatencyFrames();

		while (Broodwar->isInGame())
		{
			timerStart();
			LOG4 << "Calling onTick...";
			CodeList::onTick();
			timerEnd();

			BWAPI::BWAPIClient.update();
			if (!BWAPI::BWAPIClient.isConnected())
			{
				LOG << "Reconnecting...";
				reconnect();
			}
		}
		
		LOG << "Calling onMatchEnd...";
		CodeList::onMatchEnd();
		
		LOG << "Game ended.\n";
	}
	return 0;
}
