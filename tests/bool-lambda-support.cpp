#include "bool-lambda-support.hpp"
#include <iostream>
#include <functional>

struct TestClass : public std::binary_function<int, int, bool>
{
    bool operator () (int x, int y) const
    {
        return x < y;
    }
};

int main()
{
    auto f = TestClass() && (TestClass() || !TestClass());
    bool result = f(10, 6);
    std::cout << "result: " << (result ? "true" : "false") << "\n";
    return 0;
}
