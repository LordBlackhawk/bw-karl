#pragma once

#include "plan/plan-item.hpp"

class SecureExpert : public AbstractExpert
{
    public:
        SecureExpert(const std::string& n, AbstractExpert* e);
        ~SecureExpert();

        void prepare() override;
        bool tick(Blackboard* blackboard) override;

    protected:
        std::string     name;
        AbstractExpert* expert;
};
