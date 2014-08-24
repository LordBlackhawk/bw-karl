#pragma once

#include "abstract-action.hpp"
#include "utils/array2d.hpp"

class FieldScannerAction : public AbstractAction
{
    public:
        FieldScannerAction();

        Status onTick(AbstractExecutionEngine* engine) override;
};
