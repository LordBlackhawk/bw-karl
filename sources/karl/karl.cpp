#include "utils/myseh.hpp"
#include "utils/log.hpp"
#include "utils/thread.hpp"
#include "engine/default-execution-engine.hpp"
#include "engine/mutex-execution-engine.hpp"
#include "engine/broodwar-actions.hpp"
#include "plan/plan-item.hpp"
#include "expert/expert-registrar.hpp"

#include <BWAPI.h>
#include <BWAPI/Client.h>
#include <BWTA.h>

#include <boost/program_options.hpp>

#include <windows.h>
#include <time.h>
#include <iostream>

using namespace BWAPI;
namespace po = boost::program_options;

namespace
{
    bool doParallel = false;

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
            {
                engine = defaultEngine = new DefaultExecutionEngine();
                for (auto unit : BWAPI::Broodwar->self()->getUnits())
                    if (unit->getType() == BWAPI::UnitTypes::Zerg_Overlord)
                {
                    AbstractAction* pre = NULL;
                    for (auto location : BWTA::getStartLocations()) {
                        pre = new MoveToPositionAction(unit, location->getPosition(), pre);
                        engine->addAction(pre);
                    }
                }

                if (doParallel)
                    engine = new MutexExecutionEngine(engine);

                blackboard = new Blackboard(engine);
                ExpertRegistrar::prepareBlackboard(blackboard);
                blackboard->prepare();

                if (doParallel)
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
                Broodwar->drawTextScreen(200, 6, "Actions: %d/%d", defaultEngine->numberOfActiveActions(), defaultEngine->numberOfActions());
            }
    };

    void reconnect()
    {
        while(!BWAPIClient.connect())
        {
            Sleep(1000);
        }
    }

    bool showhud = false;
    int speed = 0;

    void mainLoop()
    {
        LOG << "Parameter loading...";
        srand(time(NULL));

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
                Sleep(1);
            }

            LOG << "Reading terran information...";
            BWTA::readMap();
            BWTA::analyze();

            LOG << "Starting match...";
            Broodwar->sendText("Hello world from Karl 3.0!");
            Broodwar->enableFlag(Flag::UserInput);
            Broodwar->setLocalSpeed(speed);

            AI ai;
            while (Broodwar->isInGame())
            {
                if (showhud)
                    ai.drawHUD();
                if (!Broodwar->isPaused())
                    ai.tick();

                BWAPI::BWAPIClient.update();
                if (!BWAPI::BWAPIClient.isConnected())
                {
                    LOG << "Reconnecting...";
                    reconnect();
                }
            }
            LOG << "Game ended.\n";
        }
    }
}

int main(int argc, char* argv[])
{
    bool showhelp       = false;
    bool writelogfiles  = false;

    po::options_description general("General options");
    general.add_options()
            ("help",        po::bool_switch(&showhelp),                 "Show this help message.")
            ("log",         po::bool_switch(&writelogfiles),            "Write log files on windows exceptions.")
            ("hud",         po::bool_switch(&showhud),                  "Show HUD.")
            ("speed",       po::value<int>(&speed)->default_value(0),   "Set game speed (-1 = default, 0 maximum speed, ...)")
            ("parallel",    po::bool_switch(&doParallel),               "Run experts parallel to star craft.")
        ;

    po::options_description all("All options");
    all.add(general);
    all.add(ExpertRegistrar::getOptions());

    try {
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(all).run(), vm);
        po::notify(vm);
    } catch (po::error& e) {
        std::cout << e.what() << "\n\n" << all;
        return 1;
    }

    if (showhelp) {
        std::cout << all;
        return 0;
    }

    ExpertRegistrar::evaluateOptions();

    seh::Registrar handler(argv[0], writelogfiles ? "./logs/" : NULL);
    try {
        mainLoop();
    } catch (std::exception& e) {
        LOG << "Catch exception: " << e.what();
    }
    return 0;
}
