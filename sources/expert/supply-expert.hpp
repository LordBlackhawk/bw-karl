#pragma once

#include "basic-expert.hpp"

#include <vector>

class SupplyExpert : public BasicPortExpert
{
    public:
        void endTraversal() override;

        void visitSupplyPort(SupplyPort* port) override;

    protected:
        std::vector<SupplyPort*> provideTerran;
        std::vector<SupplyPort*> requireTerran;
        std::vector<SupplyPort*> provideZerg;
        std::vector<SupplyPort*> requireZerg;
        std::vector<SupplyPort*> provideProtoss;
        std::vector<SupplyPort*> requireProtoss;
        bool process(std::vector<SupplyPort*>& provide, std::vector<SupplyPort*>& require);
};
