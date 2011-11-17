#include "newplan/bwplan.h"
#include "newplan/parameter-reader.h"
#include "newplan/resourcenextractor.h"
#include "newplan/displayres.h"
#include "micro/micro.h"

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

		std::cout << "Reading terran information...\";
		BWTA::readMap();
		BWTA::analyze();

		std::cout << "Starting match...\n";
		BWPlan plan = initplan;
		Mirco::instance().clear();
		Broodwar->sendText("Hello world from Karl!");
		Broodwar->enableFlag(Flag::UserInput);

		if (plan.race != Broodwar->self()->getRace()->getName()) {
			std::cout << "Error: Game started with wrong race!\n";
			return 1;
		}

		while(Broodwar->isInGame())
		{
			Broodwar->drawTextScreen(300, 0, "FPS: %f", Broodwar->getAverageFPS());

			BWResources res = extractResources();
			plan.rebase_sr(1, res);
			plan.execute();
			displayResources(res);

			Micro::instance().tick();

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