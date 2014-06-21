#include "utils/timer.hpp"
#include "utils/log.hpp"
#include "engine/default-execution-engine.hpp"
#include "engine/broodwar-events.hpp"
#include "plan/plan-item.hpp"
#include "expert/expert-registrar.hpp"

#include <BWAPI.h>
#include <BWAPI/Client.h>
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

AbstractExecutionEngine* prepareExecutionEngine()
{
    DefaultExecutionEngine* engine = new DefaultExecutionEngine();
    return engine;
}

Blackboard* prepareBlackboard(AbstractExecutionEngine* engine)
{
    Blackboard* blackboard = new Blackboard(engine);
    ExpertRegistrar::preapreBlackboard(blackboard);
    blackboard->prepare();
    return blackboard;
}

void sendFrameEvent(AbstractExecutionEngine* engine)
{
    BWAPI::Player* self = BWAPI::Broodwar->self();
    engine->generateEvent(new FrameEvent(BWAPI::Broodwar->getFrameCount(), self->minerals(), self->gas()));

    for (auto event : Broodwar->getEvents())
        engine->generateEvent(new BroodwarEvent(event));

    for (auto unit : self->getUnits())
        engine->generateEvent(new UnitEvent(unit, unit->getType(), unit->getPosition()));
}

int main(int /*argc*/, char* */*argv[]*/)
{
    LOG << "Parameter loading...";
    srand(time(NULL));
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
        Broodwar->sendText("Hello world from Karl 3.0!");
        Broodwar->enableFlag(Flag::UserInput);
        Broodwar->setLocalSpeed(0);

        AbstractExecutionEngine* engine = prepareExecutionEngine();
        Blackboard* blackboard = prepareBlackboard(engine);

        while (Broodwar->isInGame())
        {
            if (!Broodwar->isPaused()) {
                timerStart();
                sendFrameEvent(engine);
                blackboard->tick();
                engine->tick();
                timerEnd();
            }

            BWAPI::BWAPIClient.update();
            if (!BWAPI::BWAPIClient.isConnected())
            {
                LOG << "Reconnecting...";
                reconnect();
            }
        }

        delete blackboard;
        delete engine;
        LOG << "Game ended.\n";
    }
    return 0;
}
