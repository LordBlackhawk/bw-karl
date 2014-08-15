#pragma once

#include "basic-expert.hpp"

class WebGUIExpert : public BasicExpert
{
    public:
        WebGUIExpert();
        ~WebGUIExpert();

        bool tick(Blackboard* blackboard) override;
        inline Blackboard *getCurrentBlackboard() { return currentBlackboard; }
        
        static bool enabled;
        static bool isApplicable(Blackboard* blackboard);
        
        static void initialize();
        static void quit();
        static void preGameTick();
        
        static WebGUIExpert *instance();
    protected:
};
