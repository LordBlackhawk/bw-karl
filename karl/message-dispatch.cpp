// ToDo:
//  * 

#include "message-dispatch.hpp"
#include "code-list.hpp"
#include <BWAPI.h>

using namespace BWAPI;

namespace
{
    void execute()
    {
        for(auto it : Broodwar->getEvents())
        {
            switch(it.getType())
            {
                case EventType::MatchEnd:
                    CodeList::onMatchEndMessage(it.isWinner());
                    break;
                case EventType::SendText:
                    CodeList::onSendText(it.getText());
                    break;
                case EventType::ReceiveText:
                    CodeList::onReceiveText(it.getPlayer(), it.getText());
                    break;
                case EventType::PlayerLeft:
                    CodeList::onPlayerLeft(it.getPlayer());
                    break;
                case EventType::NukeDetect:
                    CodeList::onNukeDetect(it.getPosition());
                    break;
                case EventType::UnitCreate:
                    CodeList::onUnitCreate(it.getUnit());
                    break;
                case EventType::UnitDestroy:
                    CodeList::onUnitDestroy(it.getUnit());
                    break;
                case EventType::UnitMorph:
                    CodeList::onUnitMorph(it.getUnit());
                    break;
                case EventType::UnitShow:
                    CodeList::onUnitShow(it.getUnit());
                    break;
                case EventType::UnitHide:
                    CodeList::onUnitHide(it.getUnit());
                    break;
                case EventType::UnitRenegade:
                    CodeList::onUnitRenegade(it.getUnit());
                    break;
                default:
                    break;
            }
        }
    }
}

void MessageDispatchCode::onMatchBegin()
{
    //execute();
}

void MessageDispatchCode::onTick()
{
    if (Broodwar->getFrameCount() != 0)
        execute();
}
