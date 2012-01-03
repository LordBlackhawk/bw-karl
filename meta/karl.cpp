#include "utils/debug-broodwar.h"
#include "utils/debug-1.h"

#include "expandplan.h"

#include "newplan/bwplan.h"
#include "newplan/parameter-reader.h"
#include "newplan/resourcenextractor.h"
#include "newplan/displayres.h"
#include "newplan/stream-output.h"
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
	LOG << "Parsing arguments...";
	BWPlan initplan = reader.getStartPlan();
	LOG1 << "InitialPlan has " << initplan.scheduledCount() << " elements.";

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
		BWPlan plan = initplan;
		Micro::instance().clear();
		Micro::instance().prepareMap();
		Broodwar->sendText("Hello world from Karl!");
		Broodwar->enableFlag(Flag::UserInput);
		Broodwar->setLocalSpeed(0);
		
		LOG1 << "Latency Frames: " << BWAPI::Broodwar->getRemainingLatencyFrames();

		/*
		if (plan.race != Broodwar->self()->getRace()->getName()) {
			LOG << "Error: Game started with wrong race!";
			return 1;
		}
		*/

		while (Broodwar->isInGame())
		{
			Broodwar->drawTextScreen(300, 0, "FPS: %f", Broodwar->getAverageFPS());
			
			BWResources res = extractResources();
			displayResources(res);
			Micro::instance().pretick();
			
			//BWResources old = plan.startResources();
			//if (res != old) {
			//	//LOG1 << outResources(res) << "\n\tplanned: " << outResources(initplan.at(BWAPI::Broodwar->getFrameCount()).getResources());
			//	LOG1 << outResources(res) << "\n\told: " << outResources(old);
			//	if (plan.scheduledOperations().size() > 0)
			//		LOG1 << "Next in plan: " << plan.scheduledOperations().begin()->getName();
			//}
			
			LOG4 << "Rebasing plan...";
			plan.rebase_sr(1, res);
			
			LOG4 << "Updating plan...";
			updatePlan(plan);
			
			LOG4 << "Optimizing plan...";
			plan.optimizeEndTime(1);

			LOG4 << "Executing plan...";
			plan.execute();
			
			LOG4 << "Micro tick...";
			Micro::instance().tick();

			BWAPI::BWAPIClient.update();
			if (!BWAPI::BWAPIClient.isConnected())
			{
				LOG << "Reconnecting...";
				reconnect();
			}
		}
		LOG << "Game ended.\n";
	}
	return 0;
}