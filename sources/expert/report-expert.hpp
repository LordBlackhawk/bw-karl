#pragma once

#include "basic-expert.hpp"

namespace boost
{
    namespace program_options
    {
        class options_description;
    }
}

class ReportExpert final : public BasicExpert
{
    public:
        ReportExpert();
        ~ReportExpert();

        bool tick(Blackboard* blackboard) override;

        static bool isApplicable(Blackboard* blackboard);

        static boost::program_options::options_description getOptions();

    protected:
        int nextReportTime;
        std::string prefix;
};
