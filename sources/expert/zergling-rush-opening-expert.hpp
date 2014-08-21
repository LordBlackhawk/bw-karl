#pragma once

#include "basic-expert.hpp"

class ZerglingRushOpeningExpert final : public BasicPortExpert
{
    public:
        void beginTraversal() override;
        void endTraversal() override;

        void visitProvideUnitPort(ProvideUnitPort* port) override;
        void visitOwnUnitBoundaryItem(OwnUnitBoundaryItem* item) override;


    protected:
        bool poolPlanned;
        bool poolReady;
        bool haveFreeLarva;
        int workerCount;
};
