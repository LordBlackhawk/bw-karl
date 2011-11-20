#include "newplan/bwplan.h"
#include "newplan/parameter-reader.h"
#include "newplan/resourcenextractor.h"
#include "newplan/displayres.h"
#include "newplan/stream-output.h"
#include "micro/micro.h"
//#include "newplan/nocheckpoints.h"

#include <stdio.h>

#include <BWAPI.h>
#include <BWAPI\Client.h>
#include <BWTA.h>

#include <windows.h>

#include <string>

using namespace BWAPI;

void reconnect()
{
	while(!BWAPIClient.connect())
	{
		Sleep(1000);
	}
}
int main(int argc, const char* argv[])
{
	BWParameterReader reader;
	try {
		reader.run(argc, argv);
	} catch (std::exception& e) {
		std::cerr << "Error occurred while parsing parameter: " << e.what();
		return 1;
	}
	if (reader.showhelp) {
		std::cerr << "Use: Karl [Options...] [Operations...]\n" << reader;
		return 1;
	}
	std::cout << "Parsing arguments...\n";
	BWPlan initplan = reader.getStartPlan();
	std::cout << "InitialPlan has " << initplan.scheduledCount() << " elements.\n";
	std::cout << "\n";

	BWAPI::BWAPI_init();
	std::cout << "Connecting...\n";
	reconnect();
	while(true)
	{
		std::cout << "Waiting to enter match...\n";
		while (!Broodwar->isInGame())
		{
			BWAPI::BWAPIClient.update();
			if (!BWAPI::BWAPIClient.isConnected())
			{
				std::cout << "Reconnecting...\n";
				reconnect();
			}
		}

		std::cout << "Reading terran information...\n";
		BWTA::readMap();
		BWTA::analyze();

		std::cout << "Starting match...\n";
		BWPlan plan = initplan;
		Micro::instance().clear();
		Micro::instance().prepareMap();
		Broodwar->sendText("Hello world from Karl!");
		Broodwar->enableFlag(Flag::UserInput);
		Broodwar->setLocalSpeed(0);
		
		std::clog << "Latency Frames: " << BWAPI::Broodwar->getRemainingLatencyFrames() << "\n";

		/*
		if (plan.race != Broodwar->self()->getRace()->getName()) {
			std::cout << "Error: Game started with wrong race!\n";
			return 1;
		}
		*/

		while(Broodwar->isInGame())
		{
			Broodwar->drawTextScreen(300, 0, "FPS: %f", Broodwar->getAverageFPS());
			
			BWResources old = plan.startResources();
			BWResources res = extractResources();
			displayResources(res);
			Micro::instance().pretick();
			
			if (res != old)
				std::clog << BWAPI::Broodwar->getFrameCount() << ": " << outResources(res);
			
			std::clog << BWAPI::Broodwar->getFrameCount() << ": Rebasing plan...\n";
			plan.rebase_sr(1, res);

			std::clog << BWAPI::Broodwar->getFrameCount() << ": Executing plan...\n";
			plan.execute();
			
			std::clog << BWAPI::Broodwar->getFrameCount() << ": Micro tick...\n";
			Micro::instance().tick();

			//std::clog << "waiting for next frame...\n";
			BWAPI::BWAPIClient.update();
			if (!BWAPI::BWAPIClient.isConnected())
			{
				printf("Reconnecting...\n");
				reconnect();
			}
		}
		printf("Game ended.\n");
	}
	return 0;
}