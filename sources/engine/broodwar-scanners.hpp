#pragma once

#include "abstract-action.hpp"
#include "utils/array2d.hpp"

class CreepScannerAction : public AbstractAction
{
    public:
        CreepScannerAction();

        void onBegin(AbstractExecutionEngine* engine) override;
        Status onTick(AbstractExecutionEngine* engine) override;

    protected:
        Array2d<bool> fields;
};
