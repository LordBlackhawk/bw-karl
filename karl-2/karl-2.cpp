#include "utils/debug-broodwar.h"
#include "utils/debug-1.h"

#include "average-calculator.hpp"

#include "code-list.hpp"
#include "strategie.hpp"

#include <stdio.h>

#include <BWAPI.h>
#include <BWAPI\Client.h>
#include <BWTA.h>

#include <windows.h>

#include <string>
#include <cmath>

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

		LARGE_INTEGER frequency;
		if (!QueryPerformanceFrequency(&frequency))
			LOG << "No high resolution timer!";
		AverageCalculator<100, int, double> avg(1000000.0 / double(frequency.QuadPart));
		while (Broodwar->isInGame())
		{
			LARGE_INTEGER counter_start, counter_end;
			QueryPerformanceCounter(&counter_start);

			LOG4 << "Calling onTick...";
			CodeList::onTick();
			
			Broodwar->drawTextScreen(300, 0, "FPS: %f", Broodwar->getAverageFPS());
			
			QueryPerformanceCounter(&counter_end);
			avg.push(counter_end.QuadPart - counter_start.QuadPart);
			Broodwar->drawTextScreen(2, 2, "Reaction time: %.0f / %.0f / %.0f ns", avg.min(), avg.average(), avg.max());

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