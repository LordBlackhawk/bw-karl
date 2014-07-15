#include "utils/myseh.hpp"
#include "utils/timer.hpp"
#include "utils/log.hpp"
#include "engine/default-execution-engine.hpp"
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

                for (auto unit : BWAPI::Broodwar->self()->getUnits())
                    if (unit->getType() == BWAPI::UnitTypes::Zerg_Overlord)
                {
                    AbstractAction* pre = NULL;
                    for (auto location : BWTA::getStartLocations()) {
                        pre = new MoveToPositionAction(unit, location->getPosition(), pre);
                        engine->addAction(pre);
                    }
                }
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

    bool showhud = false;
    int speed = 0;

    void mainLoop()
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
            Broodwar->setLocalSpeed(speed);

            AI ai;
            while (Broodwar->isInGame())
            {
                int time = Broodwar->getFrameCount() / 24;
                Broodwar->drawTextScreen(280, 6, "%02d:%02d:%02d", time/3600, (time/60)%60, time%60);
                Broodwar->drawTextScreen(340, 6, "fps: %d", Broodwar->getFPS());

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
