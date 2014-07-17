#include "thread.hpp"

//namespace
//{
    DWORD WINAPI callThread(void* threadPtr)
    {
        Thread* thread = static_cast<Thread*>(threadPtr);
        thread->running = true;
        thread->run();
        thread->running = false;
        return 0;
    }
//}

Thread::Thread()
    : running(false), shouldTerminate(false)
{
    handle = CreateThread(NULL, 0, &callThread, this, 0, &tid);
}

Thread::~Thread()
{
    if (running)
        terminate();
    CloseHandle(handle);
}

void Thread::resume()
{
    ResumeThread(handle);
}

void Thread::suspend()
{
    SuspendThread(handle);
}

void Thread::terminate()
{
    shouldTerminate = true;
    WaitForSingleObject(handle, 2000);
}

Mutex::Mutex()
{
    InitializeCriticalSection(&critSection);
}

Mutex::~Mutex()
{
    DeleteCriticalSection(&critSection);
}

void Mutex::acquire()
{
    EnterCriticalSection(&critSection);
}

void Mutex::release()
{
    LeaveCriticalSection(&critSection);
}

Lock::Lock(Mutex& m)
    : mutex(m)
{
    mutex.acquire();
}

Lock::~Lock()
{
    mutex.release();
}
