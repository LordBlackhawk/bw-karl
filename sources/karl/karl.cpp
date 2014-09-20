#include "utils/myseh.hpp"
#include "utils/log.hpp"
#include "utils/thread.hpp"
#include "utils/options.hpp"
#include "engine/default-execution-engine.hpp"
#include "engine/mutex-execution-engine.hpp"
#include "engine/broodwar-actions.hpp"
#include "plan/plan-item.hpp"
#include "plan/broodwar-plan-items.hpp"
#include "plan/broodwar-boundary-items.hpp"
#include "expert/expert-registrar.hpp"
#include "expert/webgui-expert.hpp"

#include <BWAPI.h>
#include <BWAPI/Client.h>
#include <BWTA.h>

#include <windows.h>
#include <time.h>
#include <iostream>

using namespace BWAPI;

namespace
{
    class ExpertThread : public Thread
    {
        public:
            ExpertThread(Blackboard* b)
                : blackboard(b)
            { }

            void run() override
            {
                while (!isTerminated()) {
                    blackboard->tick();
                    Sleep(1);
                }
            }

        private:
            Blackboard* blackboard;
    };

    class AI
    {
        public:
            DefaultExecutionEngine*     defaultEngine;
            AbstractExecutionEngine*    engine;
            Blackboard*                 blackboard;
            ExpertThread*               thread;

            AI()
                : thread(NULL)
            {
                engine = defaultEngine = new DefaultExecutionEngine();

                if (OptionsRegistrar::optParallel())
                    engine = new MutexExecutionEngine(engine);

                blackboard = new Blackboard(engine);
                ExpertRegistrar::prepareBlackboard(blackboard);
                blackboard->prepare();

                if (OptionsRegistrar::optParallel())
                    thread = new ExpertThread(blackboard);
            }

            ~AI()
            {
                if (thread != NULL)
                    delete thread;
                delete blackboard;
                delete engine;
            }

            void tick()
            {
                Blackboard::sendFrameEvent(engine);
                if (thread == NULL)
                    blackboard->tick();
                engine->tick();
            }

            void drawHUD()
            {
                int time = Broodwar->getFrameCount() / 24;
                Broodwar->drawTextScreen(280, 6, "%02d:%02d:%02d", time/3600, (time/60)%60, time%60);
                Broodwar->drawTextScreen(340, 6, "fps: %d", Broodwar->getFPS());
                Broodwar->drawTextScreen(160, 6, "Actions: %d/%d", defaultEngine->numberOfActiveActions(), defaultEngine->numberOfActions());

                auto set = Broodwar->getSelectedUnits();
                if (set.size() == 1U) {
                    auto selectedUnit = *set.begin();
                    #define OUTPUT(position, function) Broodwar->drawTextScreen(10, position, #function ": %s", selectedUnit->function() ? "true" : "false");
                    OUTPUT(100, isIdle);
                    OUTPUT(115, isStartingAttack);
                    OUTPUT(130, isAttacking);
                    OUTPUT(145, isAttackFrame);
                    OUTPUT(160, isMoving);
                    OUTPUT(175, isBraking);
                    #undef OUTPUT
                } else {
                    std::map<BWAPI::UnitType, int> numbers;
                    for (auto it : BWAPI::Broodwar->self()->getUnits()) {
                        auto n = numbers.find(it->getType());
                        if (n != numbers.end()) {
                            n->second += 1;
                        } else {
                            numbers[it->getType()] = 1;
                        }
                    }
                    int top = 100;
                    for (auto it : numbers) {
                        Broodwar->drawTextScreen(10, top, "%s: %d", it.first.c_str(), it.second);
                        top += 15;
                    }
                }
            }
    };

    void reconnect()
    {
        while(!BWAPIClient.connect())
        {
            Sleep(1000);
        }
    }

    void broodwarUpdate()
    {
        BWAPI::BWAPIClient.update();
        if (!BWAPI::BWAPIClient.isConnected()) {
            LOG << "Reconnecting...";
            reconnect();
        }
    }

    void mainLoop()
    {
        LOG << "Parameter loading...";
        srand(time(NULL));

        LOG << "Setup of BWAPI...";
        BWAPI::BWAPI_init();

        WebGUIExpert::initialize();

        LOG << "Connecting...";
        reconnect();

        while(true) {
            LOG << "Waiting to enter match...";
            while (!Broodwar->isInGame()) {
                broodwarUpdate();
                WebGUIExpert::preGameTick();
                Sleep(1);
            }

            LOG << "Reading terrain information...";
            BWTA::readMap();
            BWTA::analyze();

            LOG << "Starting match...";
            Broodwar->sendText("Hello world from Karl 3.0!");
            Broodwar->enableFlag(Flag::UserInput);
            Broodwar->setLocalSpeed(OptionsRegistrar::optSpeed());

            AI ai;
            while (Broodwar->isInGame()) {
                if (OptionsRegistrar::optHUD())
                    ai.drawHUD();
                if (Broodwar->isPaused()) {
                    if (!WebGUIExpert::pauseGame)
                        Broodwar->resumeGame();
                } else {
                    ai.tick();
                    if (WebGUIExpert::pauseGame)
                        Broodwar->pauseGame();
                }
                broodwarUpdate();
            }
            LOG << "Game ended.\n";
        }

        WebGUIExpert::quit(); //FIXME: won't get here anyways...
    }
}

int main(int argc, char* argv[])
{
    OptionsRegistrar::evaluateOptions(argc, argv);

    seh::Registrar handler(argv[0], OptionsRegistrar::optLog() ? "./logs/" : NULL);
    try {
        mainLoop();
    } catch (std::exception& e) {
        LOG << "Catch exception: " << e.what();
    }
    return 0;
}
