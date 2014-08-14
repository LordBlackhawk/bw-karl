#include "webgui-expert.hpp"
#include "utils/log.hpp"
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
    struct mg_server *server;

    int event_handler(struct mg_connection *conn, enum mg_event ev) 
    {
        if (ev == MG_AUTH)
            return MG_TRUE;   // Authorize all requests

        if (ev == MG_REQUEST) 
            if(((WebGUIExpert*)conn->server_param)->handleWebRequest(conn))
                return MG_TRUE;   // Mark as processed

        return MG_FALSE;  // Rest of the events are not processed
    }

    bool get_request_int(struct mg_connection *conn, const char* name, int *destination)
    {
        char tmp[256];
        if( mg_get_var(conn, name, tmp, sizeof(tmp))>0 && 
                sscanf(tmp,"%i",destination)==1)
            return true;
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


    class WebGUIOutputVisitor : public AbstractVisitor
    {
        public:

            WebGUIOutputVisitor(mg_connection *conn) : conn(conn)
            { }

            void visitAbstractPort(AbstractPort* port)
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

            void visitAbstractBoundaryItem(AbstractBoundaryItem* item)
            {
                mg_printf_data(conn,"\"id\":\"%p\",\"unitID\":\"%i\",",
                        item,
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

            void visitAbstractPlanItem(AbstractPlanItem* item)
            {
                mg_printf_data(conn,"\"id\":\"%p\",\"active\":%s,\"impossible\":%s,\"estimatedStartTime\":\"%i\",",
                        item,
                        item->isActive()?"true":"false",
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

        private:
            mg_connection *conn;
    };
    
}


WebGUIExpert::WebGUIExpert()
{
    server = mg_create_server(this, event_handler);
    mg_set_option(server, "document_root", "webroot");      // Serve current directory
    mg_set_option(server, "listening_port", "8080");  // Open port 8080
}

WebGUIExpert::~WebGUIExpert()
{
  mg_destroy_server(&server);
}

bool WebGUIExpert::enabled = false;
bool WebGUIExpert::isApplicable(Blackboard* /*blackboard*/)
{
    return WebGUIExpert::enabled;
}
    
bool WebGUIExpert::handleWebRequest(mg_connection* conn) 
{
    if(strcmp(conn->uri,"/status") == 0)
    {
        if(!currentBlackboard)
            return false;
        
        mg_send_header(conn, "Content-Type", "application/json");
        
        mg_printf_data(conn, "{");
        
        mg_printf_data(conn, "\"time\":\"%i\",",currentBlackboard->getInformations()->lastUpdateTime);
        mg_printf_data(conn, "\"minerals\":\"%i\",",currentBlackboard->getInformations()->currentMinerals);
        mg_printf_data(conn, "\"gas\":\"%i\"",currentBlackboard->getInformations()->currentGas);
        
        mg_printf_data(conn, "}");
        
        return true;
    }
    else if(strcmp(conn->uri,"/plan") == 0)
    {
        int count;
        
        if(!currentBlackboard)
            return false;
        
        mg_send_header(conn, "Content-Type", "application/json");
        
        mg_printf_data(conn, "{");
        mg_printf_data(conn, "\"time\":\"%i\",",currentBlackboard->getInformations()->lastUpdateTime);
        mg_printf_data(conn, "\"items\":[");
        count=0;
        WebGUIOutputVisitor outputVisitor(conn);
        for(auto it:currentBlackboard->getItems())
        {
            if(count>0)
                mg_printf_data(conn,",");
            
            mg_printf_data(conn, "{");
            it->acceptVisitor(&outputVisitor);
            mg_printf_data(conn, "}");
            
            count++;
        }
        mg_printf_data(conn, "],");

        mg_printf_data(conn, "\"boundaries\":[");
        count=0;
        for(auto bound_it:currentBlackboard->getBoundaries())
        {
            AbstractBoundaryItem *it=bound_it.second;

            if(count>0)
                mg_printf_data(conn,",");
            
            mg_printf_data(conn, "{");
            it->acceptVisitor(&outputVisitor);
            mg_printf_data(conn, "}");
            
            count++;
        }
        
        mg_printf_data(conn, "],");
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
        mg_printf_data(conn, "}");
        return true;
    }
    else if(strcmp(conn->uri,"/gamespeed")==0)
    {
        int speed=0;
        
        mg_send_header(conn, "Content-Type", "text/plain");
        
        if(get_request_int(conn,"value",&speed))
        {
            BWAPI::Broodwar->setLocalSpeed(speed);
            
            LOG << "WebGUIExpert: setLocalSpeed("<<speed<<");";
            
            mg_printf_data(conn,"ok");
        }
        else
        {
            mg_printf_data(conn,"err");
        }
        
        return true;
    }
    return false;
}

void WebGUIExpert::prepare()
{
    
}

bool WebGUIExpert::tick(Blackboard* blackboard)
{
    currentBlackboard=blackboard;
    mg_poll_server(server, 0);
    currentBlackboard=NULL;
    return true;
}
