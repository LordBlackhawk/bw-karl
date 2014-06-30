#include "utils/myseh.hpp"
#include "utils/timer.hpp"
#include "utils/log.hpp"
#include "engine/default-execution-engine.hpp"
#include "plan/plan-item.hpp"
#include "expert/expert-registrar.hpp"

#include <BWAPI.h>
#include <BWAPI/Client.h>
#include <BWTA.h>

#include <windows.h>
#include <time.h>
#include <iostream>

using namespace BWAPI;

namespace
{
    class AI
    {
        public:
            AbstractExecutionEngine* engine;
            Blackboard*              blackboard;

            AI()
            {
                engine = new DefaultExecutionEngine();
                blackboard = new Blackboard(engine);
                ExpertRegistrar::preapreBlackboard(blackboard);
                blackboard->prepare();
            }

            ~AI()
            {
                delete blackboard;
                delete engine;
            }
            
            void tick()
            {
                Blackboard::sendFrameEvent(engine);
                blackboard->tick();
                engine->tick();
            }
    };

    void reconnect()
    {
        while(!BWAPIClient.connect())
        {
            Sleep(1000);
        }
    }
}

int main(int /*argc*/, char* argv[])
{
    seh::Registrar handler(argv[0], "./logs/");

    LOG << "Parameter loading...";
    srand(time(NULL));
    timerInit();

    LOG << "Setup of BWAPI...";
    BWAPI::BWAPI_init();

    LOG << "Connecting...";
    reconnect();
    try {
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
            Broodwar->sendText("Hello world from Karl 3.0!");
            Broodwar->enableFlag(Flag::UserInput);
            Broodwar->setLocalSpeed(0);

            AI ai;
            while (Broodwar->isInGame())
            {
                if (!Broodwar->isPaused()) {
                    timerStart();
                    ai.tick();
                    timerEnd();
                }

                BWAPI::BWAPIClient.update();
                if (!BWAPI::BWAPIClient.isConnected())
                {
                    LOG << "Reconnecting...";
                    reconnect();
                }
            }
            LOG << "Game ended.\n";
        }
    } catch (std::exception e) {
        LOG << "Catch exception: " << e.what();
    }
    return 0;
}
