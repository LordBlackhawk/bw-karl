#include "utils/timer.hpp"
#include "utils/log.hpp"

#include <BWAPI.h>
#include <BWAPI\Client.h>
#include <BWTA.h>

#include <windows.h>
#include <time.h>
#include <iostream>

using namespace BWAPI;

void reconnect()
{
    while(!BWAPIClient.connect())
    {
        Sleep(1000);
    }
}

int main(int /*argc*/, char* */*argv[]*/)
{
    LOG << "Parameter loading...";
    srand (time(NULL));
    timerInit();

    LOG << "Setup of BWAPI...";
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

        //LOG << "Calling onMatchBegin...";
        //CodeList::onMatchBegin();

        while (Broodwar->isInGame())
        {
            if (!Broodwar->isPaused()) {
                timerStart();
                //DEBUG << "Calling onTick...";
                //CodeList::onTick();
                timerEnd();
            } else {
                //DEBUG << "Calling onPausedTick...";
                //CodeList::onPausedTick();
            }

            BWAPI::BWAPIClient.update();
            if (!BWAPI::BWAPIClient.isConnected())
            {
                LOG << "Reconnecting...";
                reconnect();
            }
        }

        //LOG << "Calling onMatchEnd...";
        //CodeList::onMatchEnd();

        //LOG << "Checking for memory leaks...";
        //CodeList::onCheckMemoryLeaks();

        LOG << "Game ended.\n";
    }
    return 0;
}
