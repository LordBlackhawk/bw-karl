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
        static int  port;
        static bool pauseGame;
        static bool isApplicable(Blackboard* blackboard);
        
        static void initialize();
        static void quit();
        static void preGameTick();

        static void interruptEngineExecution(Blackboard* blackboard);
        
        static WebGUIExpert *instance();

    protected:
        void basicTick(Blackboard* blackboard);
};
