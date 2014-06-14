#include <boost/test/unit_test.hpp>

/* Other includes */

namespace
{
    /* Define local structs/classes/functions */
}

BOOST_AUTO_TEST_SUITE( test_suite_name )

BOOST_AUTO_TEST_CASE( test_name )
{
    /* Check a condition and quit if not fulfilled. */
    BOOST_REQUIRE( 2 > 1 );
    BOOST_REQUIRE_EQUAL( 0, NULL );

    /* Check a condition but continue if not fulfilled. */
    BOOST_CHECK( true );
    BOOST_CHECK_EQUAL( 2, 1+1 );
    BOOST_CHECK_MESSAGE( true, "This is the error message." );
}

BOOST_AUTO_TEST_SUITE_END()
