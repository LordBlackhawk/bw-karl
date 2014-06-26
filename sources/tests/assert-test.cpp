#include <boost/test/unit_test.hpp>

#include "utils/assert-throw.hpp"

BOOST_AUTO_TEST_SUITE( assert_test )

BOOST_AUTO_TEST_CASE( basic )
{
    BOOST_REQUIRE_THROW(assert(false), AssertException);
}

BOOST_AUTO_TEST_SUITE_END()
