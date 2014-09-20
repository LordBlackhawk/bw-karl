#pragma once

#include "basic-expert.hpp"

class ReportExpert final : public BasicExpert
{
    public:
        ReportExpert();
        ~ReportExpert();

        bool tick(Blackboard* blackboard) override;

        static bool isApplicable(Blackboard* blackboard);

    protected:
        int nextReportTime;
        std::string prefix;
};
