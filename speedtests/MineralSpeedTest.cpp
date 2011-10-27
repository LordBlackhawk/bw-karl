#include <stdio.h>

#include <BWAPI.h>
#include <BWAPI\Client.h>

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

int main(int /*argc*/, const char**/* argv[]*/)
{
	BWAPI::BWAPI_init();
	printf("Connecting...");
	reconnect();
	while(true)
	{
		printf("waiting to enter match\n");
		while (!Broodwar->isInGame())
		{
			BWAPI::BWAPIClient.update();
			if (!BWAPI::BWAPIClient.isConnected())
			{
				printf("Reconnecting...\n");
				reconnect();
			}
		}
		printf("starting match!\n");

		Broodwar->setLocalSpeed(0);

		if (!Broodwar->isReplay())
		{
			//send each worker to the mineral field that is closest to it
			for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
			{
				if ((*i)->getType().isWorker())
				{
					Unit* closestMineral=NULL;
					for(std::set<Unit*>::iterator m=Broodwar->getMinerals().begin();m!=Broodwar->getMinerals().end();m++)
					{
						if (closestMineral==NULL || (*i)->getDistance(*m)<(*i)->getDistance(closestMineral))
							closestMineral=*m;
					}
					if (closestMineral!=NULL)
						(*i)->rightClick(closestMineral);
				}
				else if ((*i)->getType().isResourceDepot())
				{
					//if this is a center, tell it to build the appropiate type of worker
					if ((*i)->getType().getRace()!=Races::Zerg)
					{
						(*i)->train(Broodwar->self()->getRace().getWorker());
					}
					else //if we are Zerg, we need to select a larva and morph it into a drone
					{
						std::set<Unit*> myLarva=(*i)->getLarva();
						if (myLarva.size()>0)
						{
							Unit* larva=*myLarva.begin();
							larva->morph(UnitTypes::Zerg_Drone);
						}
					}
				}
			}
		}

		while(Broodwar->isInGame())
		{
			if (Broodwar->getFrameCount() % 100 == 1)
			{
				std::cout << "Average Minerals/Worker/1000*Frame: " << 1000.0/4.0 * (double)Broodwar->self()->minerals()/(double) Broodwar->getFrameCount() << "\n";
			}

			Broodwar->drawTextScreen(300,0,"FPS: %f",Broodwar->getAverageFPS());

			BWAPI::BWAPIClient.update();
			if (!BWAPI::BWAPIClient.isConnected())
			{
				printf("Reconnecting...\n");
				reconnect();
			}
		}
		printf("Game ended\n");
	}
	system("pause");
	return 0;
}
