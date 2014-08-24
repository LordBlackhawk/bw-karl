#include "webgui-expert.hpp"
#include "utils/log.hpp"
#include "utils/assert-throw.hpp"
#include "plan/broodwar-ports.hpp"
#include "plan/broodwar-plan-items.hpp"
#include "plan/broodwar-boundary-items.hpp"
#include "expert-registrar.hpp"
#include "engine/broodwar-actions.hpp"

REGISTER_EXPERT(WebGUIExpert);

#include <mongoose.hpp>

#include <stdio.h>
#include <string.h>
#include <string>
#include <typeinfo>
#include <sstream>

namespace 
{
    struct mg_server *server=NULL;
    WebGUIExpert *webguiInstance=NULL;
    bool interruptLoopRunning=false;
    bool requestInterruptLoop=false;


    bool get_request_int(struct mg_connection *conn, const char* name, int &destination)
    {
        char tmp[256];
        if( mg_get_var(conn, name, tmp, sizeof(tmp))>0 && 
                sscanf(tmp,"%i",&destination)==1)
            return true;
        else
            mg_printf_data(conn,"err: failed to read int '%s'!",name);

        return false;
    }
    
    bool get_request_string(struct mg_connection *conn, const char* name, std::string &destination)
    {
        char tmp[256];
        if( mg_get_var(conn, name, tmp, sizeof(tmp))>0 ) 
        {
            destination=tmp;
            return true;
        }
        else
            mg_printf_data(conn,"err: failed to read string '%s'!",name);
        
        return false;
    }
    
    bool get_request_unitType(struct mg_connection *conn, const char* name, BWAPI::UnitType &destination)
    {
        std::string unitType;

        if(get_request_string(conn,name,unitType))
        {
            BWAPI::UnitType ut=BWAPI::UnitTypes::getUnitType(unitType);

            if(ut!=BWAPI::UnitTypes::Unknown)
            {
                destination=ut;
                return true;
            }
            else
                mg_printf_data(conn,"err: unknown unitType: '%s'!",unitType.c_str());
        }
        else
            mg_printf_data(conn,"err: failed to read unitType '%s'!",name);
        return false;
    }

    template<class T>
    std::string t_to_string(T i)
    {
        std::stringstream ss;
        std::string s;
        ss << i;
        s = ss.str();
        return s;
    }
    std::string position2JSON(BWAPI::Position pos)
    {
        if(pos==BWAPI::Positions::Invalid)
            return "{\"x\":\"-1\",\"y\":\"-1\",\"info\":\"invalid\"}";
        if(pos==BWAPI::Positions::None)
            return "{\"x\":\"-1\",\"y\":\"-1\",\"info\":\"none\"}";
        if(pos==BWAPI::Positions::Unknown)
            return "{\"x\":\"-1\",\"y\":\"-1\",\"info\":\"unknown\"}";

        return "{\"x\":\""+t_to_string(pos.x())+"\",\"y\":\""+t_to_string(pos.y())+"\"}";
    }
    const char *status2String(AbstractPlanItem::Status status)
    {
        switch(status)
        {
            case AbstractPlanItem::Status::Planned:
                return "planned";
            case AbstractPlanItem::Status::Active:
                return "active";
            case AbstractPlanItem::Status::Executing:
                return "executing";
            case AbstractPlanItem::Status::Failed:
                return "failed";
            default:
                return "unknown";
        }
    }

    class WebGUIOutputVisitor : public AbstractVisitor
    {
        public:

            WebGUIOutputVisitor(mg_connection *conn) : conn(conn)
            { }

            void visitAbstractPort(AbstractPort* port) override
            {
                mg_printf_data(conn,"\"id\":\"%p\",\"owner\":\"%p\",\"active\":%s,\"activeConnection\":%s,\"impossible\":%s,\"type\":\"%s\",\"estimatedTime\":\"%i\"",
                    port,
                    port->getOwner(),
                    port->isActive()?"true":"false",
                    port->isActiveConnection()?"true":"false",
                    port->isImpossible()?"true":"false",
                    port->isRequirePort()?"require":"provide",
                    port->estimatedTime);
            }
            void visitProvideUnitPort(ProvideUnitPort* port) override
            {
                visitAbstractPort(port);
                mg_printf_data(conn,",\"name\":\"%s\",\"data\":{\"unitType\":\"%s\",\"unitID\":\"%i\",\"connectedPort\":\"%p\",\"connected\":%s,\"onDemand\":%s}",
                    "ProvideUnit",
                    port->getUnitType().getName().c_str(),
                    port->getUnit()?port->getUnit()->getID():0,
                    port->getConnectedPort(),
                    port->isConnected()?"true":"false",
                    port->isOnDemand()?"true":"false");
                    //FIXME: port->getPosition()
            }
            void visitRequireUnitPort(RequireUnitPort* port) override
            {
                visitAbstractPort(port);
                mg_printf_data(conn,",\"name\":\"%s\",\"data\":{\"unitType\":\"%s\",\"unitID\":\"%i\",\"connectedPort\":\"%p\",\"connected\":%s}",
                    "RequireUnit",
                    port->getUnitType().getName().c_str(),
                    port->getUnit()?port->getUnit()->getID():0,
                    port->getConnectedPort(),
                    port->isConnected()?"true":"false");
                    //FIXME: port->getPosition()
            }
            void visitResourcePort(ResourcePort* port) override
            {
                visitAbstractPort(port);
                mg_printf_data(conn,",\"name\":\"%s\",\"data\":{\"minerals\":\"%i\",\"gas\":\"%i\"}",
                    "Resource",
                    port->getMinerals(),
                    port->getGas());
            }
            void visitSupplyPort(SupplyPort* port) override
            {
                visitAbstractPort(port);
                mg_printf_data(conn,",\"name\":\"%s\",\"data\":{\"estimatedDuration\":\"%i\",\"connected\":%s,\"race\":\"%s\",\"required\":\"%i\"}",
                    "Supply",
                    port->estimatedDuration,
                    port->isConnected()?"true":"false",
                    port->getRace().c_str(),
                    port->getRequiredAmount());
            }
            void visitProvideMineralFieldPort(ProvideMineralFieldPort* port) override
            {
                visitAbstractPort(port);
                mg_printf_data(conn,",\"name\":\"%s\",\"data\":{\"estimatedDuration\":\"%i\",\"unitID\":\"%i\",\"connectedPort\":\"%p\",\"connected\":%s}",
                    "ProvideMineralField",
                    port->estimatedDuration,
                    port->getUnit()?port->getUnit()->getID():0,
                    port->getConnectedPort(),
                    port->isConnected()?"true":"false");
            }
            void visitRequireMineralFieldPort(RequireMineralFieldPort* port) override
            {
                visitAbstractPort(port);
                mg_printf_data(conn,",\"name\":\"%s\",\"data\":{\"estimatedDuration\":\"%i\",\"unitID\":\"%i\",\"connectedPort\":\"%p\",\"connected\":%s}",
                    "RequireMineralField",
                    port->estimatedDuration,
                    port->getUnit()?port->getUnit()->getID():0,
                    port->getConnectedPort(),
                    port->isConnected()?"true":"false");
            }
            void visitRequireSpacePort(RequireSpacePort* port) override
            {
                visitAbstractPort(port);
                mg_printf_data(conn,",\"name\":\"%s\",\"data\":{\"width\":\"%i\",\"height\":\"%i\",\"unitType\":\"%s\",\"connected\":%s}",
                    "RequireSpace",
                    port->getWidth(),
                    port->getHeight(),
                    port->getUnitType().c_str(),
                    port->isConnected()?"true":"false");
                    //FIXME: port->getTilePosition()
            }
            void visitProvideEnemyUnitPort(ProvideEnemyUnitPort* port) override
            {
                visitAbstractPort(port);
                mg_printf_data(conn,",\"name\":\"%s\",\"data\":{\"unitID\":\"%i\",\"unitType\":\"%s\",\"connectedPort\":\"%p\",\"connected\":%s}",
                    "ProvideEnemyUnit",
                    port->getUnit()?port->getUnit()->getID():0,
                    port->getUnit()?port->getUnit()->getType().c_str():"unknown",
                    port->getConnectedPort(),
                    port->isConnected()?"true":"false");
                    //FIXME: port->getPosition()
            }
            void visitRequireEnemyUnitPort(RequireEnemyUnitPort* port) override
            {
                visitAbstractPort(port);
                mg_printf_data(conn,",\"name\":\"%s\",\"data\":{\"unitID\":\"%i\",\"unitType\":\"%s\",\"connectedPort\":\"%p\",\"connected\":%s}",
                    "RequireEnemyUnit",
                    port->getUnit()?port->getUnit()->getID():0,
                    port->getUnit()?port->getUnit()->getType().c_str():"unknown",
                    port->getConnectedPort(),
                    port->isConnected()?"true":"false");
            }
            void visitProvideUnitExistancePort(ProvideUnitExistancePort* port) override
            {
                visitAbstractPort(port);
                mg_printf_data(conn,",\"name\":\"%s\",\"data\":{\"unitType\":\"%s\",\"connectedPort\":\"%p\",\"connected\":%s}",
                    "ProvideUnitExistance",
                    port->getUnitType().getName().c_str(),
                    port->getConnectedPort(),
                    port->isConnected()?"true":"false");
            }
            void visitRequireUnitExistancePort(RequireUnitExistancePort* port) override
            {
                visitAbstractPort(port);
                mg_printf_data(conn,",\"name\":\"%s\",\"data\":{\"unitType\":\"%s\",\"connectedPort\":\"%p\",\"connected\":%s}",
                    "RequireUnitExistance",
                    port->getUnitType().getName().c_str(),
                    port->getConnectedPort(),
                    port->isConnected()?"true":"false");
            }

            void visitAbstractBoundaryItem(AbstractBoundaryItem* item) override
            {
                mg_printf_data(conn,",\"unitID\":\"%i\",",
                        item->unit?item->unit->getID():0
                        );
                mg_printf_data(conn,"\"ports\":[");
                int pcount=0;
                for(auto p:item->ports)
                {
                    if(pcount>0)
                        mg_printf_data(conn,",");
                    mg_printf_data(conn, "{");
                    p->acceptVisitor(this);
                    mg_printf_data(conn, "}");
                    pcount++;
                }
                mg_printf_data(conn,"]");
            }
            void visitOwnUnitBoundaryItem(OwnUnitBoundaryItem* item) override
            {
                visitAbstractBoundaryItem(item);
                mg_printf_data(conn,",\"name\":\"%s\",\"data\":{\"unitType\":\"%s\",\"unitID\":\"%i\"}",
                    "OwnUnit",
                    item->getUnitType().getName().c_str(),
                    item->getUnit()?item->getUnit()->getID():0);
                    //FIXME: item->getPosition(),item->getTilePosition()
            }
            void visitResourceBoundaryItem(ResourceBoundaryItem* item) override
            {
                visitAbstractBoundaryItem(item);
                mg_printf_data(conn,",\"name\":\"%s\",\"data\":{\"unitType\":\"%s\",\"unitID\":\"%i\",\"numberOfWorkers\":\"%i\",\"mineralsLeft\":\"%i\"}",
                    "Resource",
                    item->getUnitType().getName().c_str(),
                    item->getUnit()?item->getUnit()->getID():0,
                    item->numberOfWorkers(),
                    item->mineralsLeft());
                    //FIXME: item->getTilePosition()
            }
            void visitEnemyUnitBoundaryItem(EnemyUnitBoundaryItem* item) override
            {
                visitAbstractBoundaryItem(item);
                mg_printf_data(conn,",\"name\":\"%s\",\"data\":{\"unitType\":\"%s\",\"unitID\":\"%i\"}",
                    "EnemyUnit",
                    item->getUnitType().getName().c_str(),
                    item->getUnit()?item->getUnit()->getID():0);
                    //FIXME: item->getPosition(),item->getTilePosition()
            }

            void visitAbstractPlanItem(AbstractPlanItem* item) override
            {
                mg_printf_data(conn,",\"active\":%s,\"status\":\"%s\",\"impossible\":%s,\"estimatedStartTime\":\"%i\",",
                        item->isActive()?"true":"false",
                        status2String(item->getStatus()),
                        item->isImpossible()?"true":"false",
                        item->estimatedStartTime);
                mg_printf_data(conn,"\"ports\":[");
                int pcount=0;
                for(auto p:item->ports)
                {
                    if(pcount>0)
                        mg_printf_data(conn,",");
                    mg_printf_data(conn, "{");
                    p->acceptVisitor(this);
                    mg_printf_data(conn, "}");
                    pcount++;
                }
                mg_printf_data(conn,"]");
            }
            void visitGatherMineralPlanItem(GatherMineralsPlanItem* item) override
            {
                visitAbstractPlanItem(item);
                mg_printf_data(conn,",\"name\":\"%s\",\"data\":{}",
                        "GatherMinerals");
            }
            void visitMorphUnitPlanItem(MorphUnitPlanItem* item) override
            {
                visitAbstractPlanItem(item);
                mg_printf_data(conn,",\"name\":\"%s\",\"data\":{\"unitType\":\"%s\"}",
                        "MorphUnit",
                        item->getUnitType().c_str());
            }
            void visitMoveToPositionPlanItem(MoveToPositionPlanItem* item) override
            {
                visitAbstractPlanItem(item);
                mg_printf_data(conn,",\"name\":\"%s\",\"data\":{\"pos\":%s}",
                        "MoveToPosition",
                        position2JSON(item->getPosition()).c_str());
            }
            void visitBuildPlanItem(BuildPlanItem* item) override
            {
                visitAbstractPlanItem(item);
                mg_printf_data(conn,",\"name\":\"%s\",\"data\":{\"unitType\":\"%s\"}",
                    "Build",
                    item->getUnitType().c_str());
            }
            void visitAttackUnitPlanItem(AttackUnitPlanItem* item) override
            {
                visitAbstractPlanItem(item);
                mg_printf_data(conn,",\"name\":\"%s\",\"data\":{}",
                    "AttackUnit");
            }
            void visitAttackPositionPlanItem(AttackPositionPlanItem* item) override
            {
                visitAbstractPlanItem(item);
                mg_printf_data(conn,",\"name\":\"%s\",\"data\":{\"pos\":%s}",
                        "AttackPosition",
                        position2JSON(item->getPosition()).c_str());
            }
            void visitGiveUpPlanItem(GiveUpPlanItem* item) override
            {
                visitAbstractPlanItem(item);
                mg_printf_data(conn,",\"name\":\"%s\",\"data\":{}",
                    "GiveUp");
            }

        private:
            mg_connection *conn;
    };    
    
    int event_handler(struct mg_connection *conn, enum mg_event ev) 
    {
        if (ev == MG_AUTH)
            return MG_TRUE;   // Authorize all requests

        if (ev == MG_REQUEST)
        {
            WebGUIExpert *webgui=WebGUIExpert::instance();
            Blackboard *currentBlackboard=NULL;
            if(webgui!=NULL)
            {
                currentBlackboard=webgui->getCurrentBlackboard();
                assert(currentBlackboard!=NULL);
            }
            
            if(strcmp(conn->uri,"/status") == 0)
            {
                mg_send_header(conn, "Content-Type", "application/json");

                mg_printf_data(conn, "{");

                if(webgui==NULL)
                {
                    mg_printf_data(conn, "\"status\":\"pregame\"");
                }
                else
                {
                    mg_printf_data(conn, "\"status\":\"running\",");
                    mg_printf_data(conn, "\"time\":\"%i\",",currentBlackboard->getInformations()->lastUpdateTime);
                    mg_printf_data(conn, "\"minerals\":\"%i\",",currentBlackboard->getInformations()->currentMinerals);
                    mg_printf_data(conn, "\"gas\":\"%i\"",currentBlackboard->getInformations()->currentGas);
                }

                mg_printf_data(conn, "}");

                return MG_TRUE;
            }
            else if(strcmp(conn->uri,"/unittypes") == 0)
            {
                mg_send_header(conn, "Content-Type", "application/json");

                mg_printf_data(conn, "{");

                int count=0;
                for(auto ut:BWAPI::UnitTypes::allUnitTypes())
                {
                    if(ut.isSpecialBuilding()||ut.isNeutral()||ut.isHero())
                        continue;

                    if(count>0)
                        mg_printf_data(conn, ",");
                    mg_printf_data(conn, "\"%s\":{\"race\":\"%s\",\"isBuilding\":%s,\"isFlyer\":%s}",
                        ut.getName().c_str(),
                        ut.getRace().getName().c_str(),
                        ut.isBuilding()?"true":"false",
                        ut.isFlyer()?"true":"false"
                        );
                    count++;
                }

                mg_printf_data(conn, "}");
                return MG_TRUE;
            }
            else if(strcmp(conn->uri,"/plan") == 0)
            {
                int count;

                mg_send_header(conn, "Content-Type", "application/json");

                mg_printf_data(conn, "{");
                
                if(webgui==NULL)
                {
                    mg_printf_data(conn, "\"status\":\"pregame\"");
                }
                else
                {
                    mg_printf_data(conn, "\"status\":\"running\",");
                    mg_printf_data(conn, "\"time\":\"%i\",",currentBlackboard->getInformations()->lastUpdateTime);
                    mg_printf_data(conn, "\"items\":{");
                    count=0;
                    WebGUIOutputVisitor outputVisitor(conn);
                    for(auto it:currentBlackboard->getItems())
                    {
                        AbstractPlanItem *item=it;
                        
                        if(count>0)
                            mg_printf_data(conn,",");

                        mg_printf_data(conn, "\"%p\":{\"id\":\"%p\"",item,item);
                        item->acceptVisitor(&outputVisitor);
                        mg_printf_data(conn, "}");

                        count++;
                    }
                    mg_printf_data(conn, "},");

                    mg_printf_data(conn, "\"boundaries\":{");
                    count=0;
                    for(auto bound_it:currentBlackboard->getBoundaries())
                    {
                        AbstractBoundaryItem *item=bound_it.second;

                        if(count>0)
                            mg_printf_data(conn,",");

                        
                        mg_printf_data(conn, "\"%p\":{\"id\":\"%p\"",item,item);
                        item->acceptVisitor(&outputVisitor);
                        mg_printf_data(conn, "}");

                        count++;
                    }

                    mg_printf_data(conn, "},");
                    mg_printf_data(conn, "\"actions\":[");
                    /*
                    count=0;
                    for(auto act:currentBlackboard->actionMap)
                    {

                        if(count>0)
                            mg_printf_data(conn,",");

                        AbstractAction *action=act.first;
                        AbstractPlanItem *planItem=act.second;

                        mg_printf_data(conn,"{\"id\":\"%p\",",
                            action);

                        if(CollectMineralsAction *collectMinerals=dynamic_cast<CollectMineralsAction*>(action))
                        {
                            mg_printf_data(conn,"\"name\":\"%s\",\"data\":{}",
                                    "CollectMinerals");
                        }
                        else if(ZergBuildAction *zergBuild=dynamic_cast<ZergBuildAction*>(action))
                        {
                            mg_printf_data(conn,"\"name\":\"%s\",\"data\":{}",
                                    "ZergBuild");
                        }
                        else if(MorphUnitAction *morphUnit=dynamic_cast<MorphUnitAction*>(action))
                        {
                            mg_printf_data(conn,"\"name\":\"%s\",\"data\":{}",
                                    "MorphUnit");
                        }
                        else if(MoveToPositionAction *moveToPosition=dynamic_cast<MoveToPositionAction*>(action))
                        {
                            mg_printf_data(conn,"\"name\":\"%s\",\"data\":{}",
                                    "MoveToPosition");
                        }
                        else if(AttackPositionAction *attackPosition=dynamic_cast<AttackPositionAction*>(action))
                        {
                            mg_printf_data(conn,"\"name\":\"%s\",\"data\":{}",
                                    "AttackPosition");
                        }
                        else if(AttackUnitAction *attackUnit=dynamic_cast<AttackUnitAction*>(action))
                        {
                            mg_printf_data(conn,"\"name\":\"%s\",\"data\":{}",
                                    "AttackUnit");
                        }
                        else if(SendTextAction *sendText=dynamic_cast<SendTextAction*>(action))
                        {
                            mg_printf_data(conn,"\"name\":\"%s\",\"data\":{}",
                                    "SendText");
                        }
                        else if(GiveUpAction *giveUp=dynamic_cast<GiveUpAction*>(action))
                        {
                            mg_printf_data(conn,"\"name\":\"%s\",\"data\":{}",
                                    "GiveUp");
                        }
                        else
                        {
                            mg_printf_data(conn,"\"name\":\"%s\",\"data\":{}",
                                typeid(*action).name());
                        }

                        mg_printf_data(conn,"}");
                        count++;
                    }
                     */

                    mg_printf_data(conn, "]");        
                }
                mg_printf_data(conn, "}");
                return MG_TRUE;
            }
            else if(strcmp(conn->uri,"/gamespeed")==0)
            {
                int speed=0;

                mg_send_header(conn, "Content-Type", "text/plain");

                if(get_request_int(conn,"value",speed))
                {
                    BWAPI::Broodwar->setLocalSpeed(speed);

                    LOG << "WebGUIExpert: setLocalSpeed("<<speed<<");";

                    mg_printf_data(conn,"ok");
                }

                return MG_TRUE;
            }
            else if(strcmp(conn->uri,"/interrupt")==0)
            {
                mg_send_header(conn, "Content-Type", "text/plain");

                if(!currentBlackboard)
                {
                    mg_printf_data(conn,"err: not in game.");
                }
                else
                {
                    requestInterruptLoop=true;
                    mg_printf_data(conn,"ok");
                }

                return MG_TRUE;
            }
            else if(strcmp(conn->uri,"/continue")==0)
            {
                mg_send_header(conn, "Content-Type", "text/plain");

                interruptLoopRunning=false;

                mg_printf_data(conn,"ok");

                return MG_TRUE;
            }
            else if(strcmp(conn->uri,"/add")==0)
            {
                std::string type;
                
                mg_send_header(conn, "Content-Type", "text/plain");
                
                if(!webgui)
                {
                    mg_printf_data(conn,"err: not in game.");
                    return MG_TRUE;
                }

                if(get_request_string(conn,"type",type))
                {
                    if(type=="BuildPlanItem")
                    {
                        BWAPI::UnitType ut;

                        if(get_request_unitType(conn,"unitType",ut))
                        {
                            currentBlackboard->build(ut);
                            LOG << "WebGUIExpert: add BuildPlanItem("<<ut.getName()<<");";
                            mg_printf_data(conn,"ok");
                        }
                    }
                    else if(type=="MorphUnitPlanItem")
                    {
                        BWAPI::UnitType ut;

                        if(get_request_unitType(conn,"unitType",ut))
                        {
                            currentBlackboard->morph(ut);
                            LOG << "WebGUIExpert: add MorphUnitPlanItem("<<ut.getName()<<");";
                            mg_printf_data(conn,"ok");
                        }
                    }
                    else
                        mg_printf_data(conn,"err: unknown planitem type '%s'!",type.c_str());
                }

                return MG_TRUE;
            }
        }
        return MG_FALSE;  // Rest of the events are not processed
    }
}


WebGUIExpert::WebGUIExpert()
{
    assert(webguiInstance==NULL);
    webguiInstance=this;
    initialize();
}

WebGUIExpert::~WebGUIExpert()
{
    assert(webguiInstance==this);
            
    webguiInstance=NULL;
}

bool WebGUIExpert::enabled = false;
bool WebGUIExpert::isApplicable(Blackboard* /*blackboard*/)
{
    return WebGUIExpert::enabled;
}

bool WebGUIExpert::tick(Blackboard* blackboard)
{
    currentBlackboard=blackboard;
    mg_poll_server(server, 0);
    currentBlackboard=NULL;

    if(requestInterruptLoop && !interruptLoopRunning)
        WebGUIExpert::interruptEngineExecution(blackboard);
    requestInterruptLoop=false;
    
    return true;
}

void WebGUIExpert::preGameTick()
{
    if(server!=NULL)
        mg_poll_server(server, 0);
}

#include <windows.h>
void WebGUIExpert::interruptEngineExecution(Blackboard* blackboard)
{
    interruptLoopRunning=true;
    std::cout << "\n###### execution interrupted, WebGUI active on port 8080 ######\n";
    while(interruptLoopRunning)
    {
        WebGUIExpert::instance()->tick(blackboard);
        Sleep(10);
    }
    std::cout << "\n###### continue execution ######\n";
}

void WebGUIExpert::initialize()
{
    if(WebGUIExpert::enabled && server==NULL)
    {
        server = mg_create_server(NULL, event_handler);
        mg_set_option(server, "document_root", "webroot");      // Serve current directory
        mg_set_option(server, "listening_port", "8080");  // Open port 8080
    }
}

void WebGUIExpert::quit()
{
    if(server!=NULL)
        mg_destroy_server(&server);
    server=NULL;
}

WebGUIExpert *WebGUIExpert::instance()
{
    return webguiInstance;
}