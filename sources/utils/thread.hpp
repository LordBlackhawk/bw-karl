#pragma once

#include <windows.h>

class Thread
{
    public:
        Thread();
        virtual ~Thread();
        virtual void run() = 0;
        void resume();
        void suspend();
        void terminate();
        bool isRunning() const { return running; }

    protected:
        bool isTerminated() const { return shouldTerminate; }

    private:
        HANDLE handle;
        DWORD  tid;
        bool running;
        bool shouldTerminate;
        friend DWORD WINAPI callThread(void* threadPtr);
};

class Mutex
{
    public:
        Mutex ();
        ~Mutex ();

    private:
        friend class Lock;
        CRITICAL_SECTION critSection;

        void acquire();
        void release();
};

class Lock
{
    public:
        Lock(Mutex& m);
        ~Lock();

    private:
        Mutex& mutex;
};
