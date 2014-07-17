#include <boost/test/unit_test.hpp>
#include "utils/thread.hpp"
#include <string>
#include <iostream>
#include <windows.h>

/*
namespace
{
    Mutex mutex;

    class ExampleThread : public Thread
    {
        public:
            ExampleThread(int p1, int p2)
                : param1(p1), param2(p2)
            { }

            void run() override
            {
                while(!isTerminated()) {
                    {
                        Lock lock(mutex);
                        printf("My Thread! Param1:%d, Param2:%d\n", param1, param2);
                        fflush(stdout);
                    }
                    Sleep(10);
                }
            }

        private:
            int param1, param2;
    };
}

BOOST_AUTO_TEST_SUITE( thread_test )

BOOST_AUTO_TEST_CASE( windows )
{
    ExampleThread thread(10, 12);

    for (int k=0; k<5; ++k) {
        {
            Lock lock(mutex);
            printf("Main Program!\n");
            fflush(stdout);
        }
        Sleep(20);
    }

    thread.terminate();
}

BOOST_AUTO_TEST_SUITE_END()
*/
