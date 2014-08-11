#include "utils/myseh.hpp"
#include "utils/log.hpp"
#include "utils/thread.hpp"
#include "engine/default-execution-engine.hpp"
#include "engine/mutex-execution-engine.hpp"
#include "engine/broodwar-actions.hpp"
#include "plan/plan-item.hpp"
#include "plan/broodwar-plan-items.hpp"
#include "plan/broodwar-boundary-items.hpp"
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
                : thread(NULL)
            {
                engine = defaultEngine = new DefaultExecutionEngine();

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
                // Test GiveUpAction
                static GiveUpAction *giveup=NULL;
                if(giveup==NULL)for(auto unit : BWAPI::Broodwar->getUnitsInRadius(BWTA::getStartLocation(BWAPI::Broodwar->self())->getPosition(),32*20))
                    if(unit->getPlayer()->isEnemy(BWAPI::Broodwar->self()))
                {
                    giveup=new GiveUpAction();
                    BWAPI::Broodwar->printf("enemy unit near our base!!! Giving up!");
                    engine->addAction(giveup);
                }

                if (thread == NULL) {
                    // Test MorphUnitPlanItem
                    bool already_queued_morph=false;
                    int workerCount=0;
                    for(auto it : blackboard->getBoundaries())
                    {
                        auto unit = dynamic_cast<OwnUnitBoundaryItem*>(it.second);
                        if ((unit != NULL) && (unit->getUnitType().isWorker()))
                            workerCount++;
                    }
                    auto morphType=workerCount<4 ? BWAPI::UnitTypes::Zerg_Drone : BWAPI::UnitTypes::Zerg_Zergling;
                    for(auto it:blackboard->getItems())
                    {
                        auto morph = dynamic_cast<MorphUnitPlanItem*>(it);
                        if(morph!=NULL && morph->getUnitType() == morphType)
                        {
                            if(!morph->isActive())
                                already_queued_morph=true;
                        }
                    }
                    if(!already_queued_morph)
                    {
                        LOG << "adding morph: "<<morphType.getName();
                        blackboard->addItem(new MorphUnitPlanItem(workerCount<4 ? BWAPI::UnitTypes::Zerg_Drone : BWAPI::UnitTypes::Zerg_Zergling, NULL));
                    }
                    
                    // Test MoveToPositionAction
                    if (BWAPI::Broodwar->getFrameCount() == 2) {
                        for (auto it : blackboard->getBoundaries()) {
                            auto unit = dynamic_cast<OwnUnitBoundaryItem*>(it.second);
                            if ((unit != NULL) && (unit->getUnitType() == BWAPI::UnitTypes::Zerg_Overlord)) {
                                //std::cout << "Overlord found!\n";
                                auto provideUnit = &unit->provideUnit;
                                for (auto location : BWTA::getStartLocations())
                                {
                                    auto item = new MoveToPositionPlanItem(provideUnit, location->getPosition());
                                    blackboard->addItem(item);
                                    provideUnit = &item->provideUnit;
                                }
                            }
                        }
                    }
                }

                // Test hand coded zergling rush
                static int attackDelay=0;
                if(((++attackDelay)%32)==0)
                {
                    BWAPI::Unit *enemyTarget = NULL;
                    for(auto enemy : BWAPI::Broodwar->getAllUnits())
                        if(enemy->getPlayer()->isEnemy(BWAPI::Broodwar->self()) && !enemy->getType().isFlyer() )
                    {
                            enemyTarget=enemy; //find a random enemy to attack
                    }
                    if(enemyTarget)
                        for(auto unit : BWAPI::Broodwar->self()->getUnits())
                    {
                        if(unit->getType()==BWAPI::UnitTypes::Zerg_Zergling && unit->isIdle())
                        {
                                // send idle units to attack
                            //LOG << "Added attack action.";
                            engine->addAction(new AttackPositionAction(unit,enemyTarget->getPosition()));
                        }
                    }
                    else
                    {
                        auto army=BWAPI::Broodwar->self()->getUnits();
                        auto a=army.begin();

                        for(auto location : BWTA::getStartLocations()) //check start locations first
                        {
                            if(!BWAPI::Broodwar->isExplored(location->getTilePosition())) //check if bases are explored
                            {
                                while(a!=army.end())
                                {
                                    if((*a)->getType()==BWAPI::UnitTypes::Zerg_Zergling && (*a)->isIdle())
                                    {
                                            //send next free unit to explore
                                        //LOG << "Added attack action.";
                                        engine->addAction(new AttackPositionAction(*a,location->getPosition()));
                                        a++;
                                        break;
                                    }
                                    a++;
                                }
                            }
                        }
                        for(auto location : BWTA::getBaseLocations()) //check other base locations
                            if(!location->isStartLocation() && !BWAPI::Broodwar->isExplored(location->getTilePosition())) //check if bases are explored
                            {
                                while(a!=army.end())
                                {
                                    if((*a)->getType()==BWAPI::UnitTypes::Zerg_Zergling && (*a)->isIdle())
                                    {
                                            //send next free unit to explore
                                        //LOG << "Added attack action.";
                                        engine->addAction(new AttackPositionAction(*a,location->getPosition()));
                                        a++;
                                        break;
                                    }
                                    a++;
                                }
                            }
                    }
                }

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
                    OUTPUT(275, isBraking);
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

        while(true) {
            LOG << "Waiting to enter match...";
            while (!Broodwar->isInGame()) {
                broodwarUpdate();
                Sleep(1);
            }

            LOG << "Reading terrain information...";
            BWTA::readMap();
            BWTA::analyze();

            LOG << "Starting match...";
            Broodwar->sendText("Hello world from Karl 3.0!");
            Broodwar->enableFlag(Flag::UserInput);
            Broodwar->setLocalSpeed(speed);

            AI ai;
            while (Broodwar->isInGame()) {
                if (showhud)
                    ai.drawHUD();
                if (!Broodwar->isPaused())
                    ai.tick();
                broodwarUpdate();
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
            ("parallel",    po::bool_switch(&doParallel),               "Run experts parallel to StarCraft.")
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
        ExpertRegistrar::listExperts(std::cout);
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
