#pragma once

#include "basic-expert.hpp"

class WorkerExpert : public BasicPortExpert
{
    public:
        void visitAbstractPlanItem(AbstractPlanItem* item);
        void visitProvideUnitPort(ProvideUnitPort* port);
};
