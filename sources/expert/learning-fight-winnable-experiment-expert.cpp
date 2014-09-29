#include "learning-fight-winnable-experiment-expert.hpp"
#include "expert-registrar.hpp"


#include "utils/log.hpp"
#include "utils/options.hpp"

#include "bwmapmodifier/bwmapmodifier.hpp"

#include <stdlib.h>
#include <ios>
#include <fstream>
#include <vector>

namespace
{
        //internal experiment counters
    int ownUnitCount=0;
    int enemyUnitCount=1;
    int enemyUnitTypeIndex=0;
    int experimentCount=99999999;

        //parameters:
    std::string starcraftMapPath;   //where to save autogenerated maps
    std::string experiment;         //name of current experiment
    int repetitions;                //how often to run the same experiment
    bool nodraw;                    //disable update of screen to speed up experiments

    std::string unitTypeName="";    //unit type to place on map
    BWAPI::UnitType unitType;


    const char *otherUnitTypes[]={
        "Zerg Drone",
        "Zerg Zergling",
        "Zerg Hydralisk",
        "Zerg Ultralisk",
        "Terran SCV",
        "Terran Marine",
        "Terran Firebat",
        "Terran Vulture",
        "Terran Goliath",
        "Protoss Probe",
        "Protoss Zealot",
        "Protoss Dragoon"
    };
#define NUMBER_OF_OTHER_UNITS (int)(sizeof(otherUnitTypes)/sizeof(otherUnitTypes[0]))
}



DEF_OPTIONS
{
    po::options_description options("LearningFightWinnableExperiment Options");
    options.add_options()
            ("mappath",        po::value<std::string>(&starcraftMapPath),      "Path to the Starcraft map folder.")
            ("experiment",     po::value<std::string>(&experiment),            "Experiment to run.")
            ("unittype",       po::value<std::string>(&unitTypeName),          "UnitType to place on map.")
            ("repetitions",    po::value<int>(&repetitions)->default_value(1), "How often to repeat the same experiment.")
            ("nodraw",         po::bool_switch(&nodraw),                   "Disable drawing to speed up experiments.")
        ;
    return options;
}

DEF_OPTION_EVENT(onEvaluate)
{
    // read 'experiment'-option
    if (!experiment.empty())
    {
        LOG << "LearningFightWinnableExperimentExpert:";

        if(experiment=="sametype")
        {
        }
        else if(experiment=="othertype")
        {
        }
        else
        {
            LOG<<" unknown experiment: "<<experiment;
            exit(1);
        }

        if(!starcraftMapPath.empty())
        {
            LOG<<"saving autogenerated maps to "<<starcraftMapPath;
        }
        else
        {
            LOG<<"mappath not specified!";
            exit(1);
        }

        if(unitTypeName.empty())
        {
            LOG<<"unittype not specified!";
            exit(1);
        }
        else
        {
            /* BWAPI not ready yet, can't check unitType
            unitType=BWAPI::UnitTypes::getUnitType(unitTypeName);

            if(unitType==BWAPI::UnitTypes::Unknown)
            {
                LOG<<"unknown unittype: "<<unitTypeName<<"\n";
                exit(1);
            }*/
        }

        LOG << "running experiment: "<<experiment<<" with "<<repetitions<<" repetition(s).";
    }
}

DEF_OPTION_EVENT(onHelp)
{
    std::cout << "\nExperiments:\n";
    std::cout<<"  sametype:             only units of the same type.\n";
    std::cout<<"  othertype:            test unittype against various other types.\n";
}


REGISTER_EXPERT(LearningFightWinnableExperimentExpert)

bool LearningFightWinnableExperimentExpert::isApplicable(Blackboard* /*blackboard*/)
{
    if(BWAPI::Broodwar->mapName()=="MachinelearningFightWinnable"
        && (  BWAPI::Broodwar->mapFileName()=="learn-fight-winnable-1.scx"
            ||BWAPI::Broodwar->mapFileName()=="learn-fight-winnable-2.scx"
            ||BWAPI::Broodwar->mapFileName()=="learn-fight-winnable-3.scx"
            ||BWAPI::Broodwar->mapFileName()=="learn-fight-winnable-4.scx") )
    {
        if(!experiment.empty())
        {
            LOG << "LearningFightWinnableExperimentExpert:";
            LOG << "  running experiment: "<<experiment;
            LOG << "  on "<<BWAPI::Broodwar->mapName()<<" ("<<BWAPI::Broodwar->mapPathName()<<").";

            unitType=BWAPI::UnitTypes::getUnitType(unitTypeName);

            if(unitType==BWAPI::UnitTypes::Unknown)
            {
                LOG<<"unknown unittype: "<<unitTypeName<<"\n";
                exit(1);
            }

            if(nodraw)
            {
                    //run as fast as possible
                BWAPI::Broodwar->setLocalSpeed(0);
                BWAPI::Broodwar->setGUI(false);
            }
            return true;
        }
        else
            LOG << "LearningFightWinnableExperimentExpert: no experiment specified!";
    }
    return false;
}
void LearningFightWinnableExperimentExpert::visitProvideUnitPort(ProvideUnitPort* port)
{
    if (port->isConnected() || port->estimatedTime > currentBlackboard->getLastUpdateTime()+100)
        return;

    BWAPI::TilePosition tp=currentBlackboard->self()->getStartLocation();

        //send idle units to attack the area of our starting position
    if(port->getPosition().getDistance((BWAPI::Position)tp)>32*10)
        currentBlackboard->attack(port, (BWAPI::Position)tp);
}

void LearningFightWinnableExperimentExpert::visitOwnUnitBoundaryItem(OwnUnitBoundaryItem* /*item*/)
{

}

void LearningFightWinnableExperimentExpert::visitEnemyUnitBoundaryItem(EnemyUnitBoundaryItem* /*item*/)
{

}


#define MAX_UNIT_COUNT 30
#define SKIP_OBVIOUS true

namespace
{
    void appendResultToCSV(std::string filename,int result)
    {
        std::ofstream csv(filename, std::ios_base::app | std::ios_base::out);

        for(int i=0;i<MAX_UNIT_COUNT;i++)
        {
            if(i<ownUnitCount)
                csv<<unitType.maxHitPoints()<<",";
            else
                csv<<"0,";
        }
        for(int i=0;i<MAX_UNIT_COUNT;i++)
        {
            if(i<enemyUnitCount)
                csv<<unitType.maxHitPoints()<<",";
            else
                csv<<"0,";
        }
        csv<<result<<"\n";

        LOG << "Result: "<<ownUnitCount<<" vs "<<enemyUnitCount<<": "<<result;
    }
}

void LearningFightWinnableExperimentExpert::matchEnd(Blackboard* blackboard)
{
    BWMapModifier map("bwapi-data/maps/template-fight-winnable.scx");

    if(experiment=="sametype")
    {
        if(ownUnitCount>0 && enemyUnitCount<=MAX_UNIT_COUNT)
        {
            appendResultToCSV("learning/data/sametype.csv",blackboard->getInformations()->isWinner?1:0);
        }
        else
            LOG<<"Ignoring out-of-bound dataset.";

        bool skipped=true;
        while(skipped)  //determine next experiment setup
        {
            if(++experimentCount>=repetitions)
            {
                ownUnitCount++;
                if(ownUnitCount>MAX_UNIT_COUNT)
                {
                    ownUnitCount=1;
                    enemyUnitCount++;

                    if(enemyUnitCount>MAX_UNIT_COUNT)
                    {
                        BWAPI::Broodwar->pauseGame();
                        LOG<<"all experiments done.";
                        exit(0);
                    }
                }
                experimentCount=0;
            }
            
            skipped=false;
            if(SKIP_OBVIOUS)    //report default win/los if player/enemy has way more units
            {
                if(ownUnitCount>enemyUnitCount+2+enemyUnitCount/8)
                {
                    appendResultToCSV("learning/data/sametype.csv",1);
                    skipped=true;
                }
                else if(ownUnitCount<enemyUnitCount-2-enemyUnitCount/8)
                {
                    appendResultToCSV("learning/data/sametype.csv",0);
                    skipped=true;
                }
            }
        }

        LOG << "placing "<<ownUnitCount<<" own "<<unitType.getName()<<" and "<<enemyUnitCount<<" enemy "<<unitType.getName()<<" on next map.";
        int cnt;

        for(cnt=0;cnt<ownUnitCount;cnt++)
        {
            map.addUnit(unitType,BWAPI::Position(32*32 + 32*cnt - 16*ownUnitCount,64),0);
        }
        for(cnt=0;cnt<enemyUnitCount;cnt++)
        {
            map.addUnit(unitType,BWAPI::Position(32*32 + 32*cnt - 16*enemyUnitCount,32*64-64),1);
        }
    }
    if(experiment=="othertype")
    {
        int result=blackboard->getInformations()->isWinner?1:0;
        if(ownUnitCount>0 && enemyUnitTypeIndex<NUMBER_OF_OTHER_UNITS)
        {
            std::ofstream csv("learning/data/othertype.csv", std::ios_base::app | std::ios_base::out);

            for(int i=0;i<NUMBER_OF_OTHER_UNITS;i++)
            {
                if(unitType.getName()==otherUnitTypes[i])
                    csv<<ownUnitCount<<",";
                else
                    csv<<"0,";
            }
            for(int i=0;i<NUMBER_OF_OTHER_UNITS;i++)
            {
                if(i==enemyUnitTypeIndex)
                    csv<<enemyUnitCount<<",";
                else
                    csv<<"0,";
            }
            csv<<result<<"\n";

            LOG << "Result: "<<ownUnitCount<<" "<<unitType.getName()<<" vs "<<enemyUnitCount<<" "<<otherUnitTypes[enemyUnitTypeIndex]<<": "<<result;
        }
        else
            LOG<<"Ignoring out-of-bound dataset.";

        bool skipped=true;
        while(skipped)  //determine next experiment setup
        {
            if(++experimentCount>=repetitions)
            {
                experimentCount=0;
                ownUnitCount++;
                if(ownUnitCount>6)
                {
                    ownUnitCount=1;
                    enemyUnitCount++;
                    if(enemyUnitCount>6)
                    {
                        enemyUnitCount=1;
                        enemyUnitTypeIndex++;
                        if(enemyUnitTypeIndex>=NUMBER_OF_OTHER_UNITS)
                        {
                            BWAPI::Broodwar->pauseGame();
                            LOG<<"all experiments done.";
                            exit(0);
                        }
                    }
                }
            }

            skipped=false;
            if(SKIP_OBVIOUS)    //report default win/los if player/enemy has way more units
            {
                /*
                if(ownUnitCount>enemyUnitCount+2+enemyUnitCount/8)
                {
                    appendResultToCSV("learning/data/othertype.csv",1);
                    skipped=true;
                }
                else if(ownUnitCount<enemyUnitCount-2-enemyUnitCount/8)
                {
                    appendResultToCSV("learning/data/othertype.csv",0);
                    skipped=true;
                }
                 * */
            }
        }

        LOG << "placing "<<ownUnitCount<<" own "<<unitType.getName()<<" and "<<enemyUnitCount<<" enemy "<<otherUnitTypes[enemyUnitTypeIndex]<<" on next map.";
        int cnt;

        for(cnt=0;cnt<ownUnitCount;cnt++)
        {
            map.addUnit(unitType,BWAPI::Position(32*32 + 32*cnt - 16*ownUnitCount,64),0);
        }
        for(cnt=0;cnt<enemyUnitCount;cnt++)
        {
            map.addUnit(BWAPI::UnitTypes::getUnitType(otherUnitTypes[enemyUnitTypeIndex]),BWAPI::Position(32*32 + 32*cnt - 16*enemyUnitCount,32*64-64),1);
        }
    }

            //HACK: we overwrite all 4 files (except the current map) to make sure we hit the next map
            // (BWAPI seems to only load *-2 and *-4 ?)
        if(BWAPI::Broodwar->mapFileName()!="learn-fight-winnable-1.scx")
            map.save(starcraftMapPath+"learn-fight-winnable-1.scx");
        if(BWAPI::Broodwar->mapFileName()!="learn-fight-winnable-2.scx")
            map.save(starcraftMapPath+"learn-fight-winnable-2.scx");
        if(BWAPI::Broodwar->mapFileName()!="learn-fight-winnable-3.scx")
            map.save(starcraftMapPath+"learn-fight-winnable-3.scx");
        if(BWAPI::Broodwar->mapFileName()!="learn-fight-winnable-4.scx")
            map.save(starcraftMapPath+"learn-fight-winnable-4.scx");

}
