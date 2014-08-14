#pragma once

#include "basic-expert.hpp"

class WebGUIExpert : public BasicExpert
{
    public:
        WebGUIExpert();
        ~WebGUIExpert();

        void prepare() override;
        bool tick(Blackboard* blackboard) override;
        
        static bool enabled;
        static bool isApplicable(Blackboard* blackboard);
        
        bool handleWebRequest(struct mg_connection *conn);
    protected:
};
