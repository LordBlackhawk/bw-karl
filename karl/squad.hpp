#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"
#include "unit-micromanagement.hpp"



class Squad
{
public:
    struct Line
    {
        BWAPI::Position p[2];
        
        Line(BWAPI::Position p1,BWAPI::Position p2)
        {
            p[0]=p1;
            p[1]=p2;
        }
        Line()
        {
            
        }
        
        BWAPI::Position getCenter()
        {
            return BWAPI::Position((p[0].x()+p[1].x())/2,(p[0].y()+p[1].y())/2);
        }
        
        BWAPI::Position getForward()
        {
            BWAPI::Position diff=p[1]-p[0];
            if(diff.getLength()>0)
            {
                    //normalize to length 8
                return BWAPI::Position(-diff.y()*8/diff.getLength(),diff.x()*8/diff.getLength());
            }
            return BWAPI::Position(0,0);
        }
        BWAPI::Position getSideward()
        {
            BWAPI::Position diff=p[1]-p[0];
            if(diff.getLength()>0)
            {
                    //normalize to length 8
                return BWAPI::Position(diff.x()*8/diff.getLength(),diff.y()*8/diff.getLength());
            }
            return BWAPI::Position(0,0);
        }
        
        
        double getLength()
        {
            BWAPI::Position diff=p[1]-p[0];
            return diff.getLength();
        }
    };

    BWAPI::Position getCenter();
    Line getDefendLine();
    
    int getUnitCount();
    int getUnitCount(const BWAPI::UnitType& ut);
    
    void defend(BWAPI::Position pos);
    void defend(BWAPI::Position p1,BWAPI::Position p2);
    void defend(Line d);
    
    //void addUnit(UnitMicromanagement *unit);
    void addUnit(BWAPI::Unit *unit);
    void addUnit(MicromanagedUnit *microUnit);
    
    std::string getName() const;
    void setName(std::string desc);
    
    Squad();
    virtual ~Squad();
    
    virtual void onTick();
    virtual void onDrawPlan();
    
private:
    Line defendLine;
    //std::set<UnitMicromanagement *> units;
    std::string name;
    std::set<MicromanagedUnit*> units;
    
    bool haveToRecalculatePositions;
};

Squad* getSquadByName(const std::string& name);

struct SquadCode : public DefaultCode
{
    static void onMatchBegin();
    static void onMatchEnd();
    static void onTick();
    static void onDrawPlan(HUDTextOutput& hud);
    static void onCheckMemoryLeaks();
};
