#include <boost/test/unit_test.hpp>

#include "seh-test-implementation.hpp"
#include "utils/myseh.hpp"

BOOST_AUTO_TEST_SUITE( seh_test )

BOOST_AUTO_TEST_CASE( basic_catch_exception )
{
    seh::Registrar handler;
    bool exception_catch = false;
    try {
        int* ptr = NULL;
        exfun(ptr);
    } catch (std::exception e) {
        exception_catch = true;
    }
    BOOST_CHECK( exception_catch );
}

BOOST_AUTO_TEST_CASE( catch_stack_overflow )
{
    seh::Registrar handler;
    bool exception_catch = false;
    try {
        stackoverflow(1);
    } catch (std::exception e) {
        exception_catch = true;
    }
    BOOST_CHECK( exception_catch );
}

BOOST_AUTO_TEST_CASE( virtual_function )
{
    seh::Registrar handler; //("./tests.exe", "./logs/");
    bool exception_catch = false;
    try {
        Base* b = getExfun();
        b->execute(NULL);
    } catch (std::exception e) {
        exception_catch = true;
    }
    BOOST_CHECK( exception_catch );
}


BOOST_AUTO_TEST_SUITE_END()
