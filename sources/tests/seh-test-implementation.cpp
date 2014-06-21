#include "seh-test-implementation.hpp"

void stackoverflow(int k)
{
    if (k != 0)
        stackoverflow(k + 1);
    stackoverflow(k - 1);
}

int exfun(int* x)
{
    int result = 50*(*x)+1;
    return (result > 20) ? (result < 40) ? result : *x : 20;
}

namespace
{
    class GetExfanClass : public Base
    {
        public:
            int execute(int* x)
            {
                return exfun(x);
            }
    };
}

Base* getExfun()
{
    return new GetExfanClass();
}
