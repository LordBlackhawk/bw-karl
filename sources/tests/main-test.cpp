#define BOOST_TEST_MODULE Test
#include <boost/test/unit_test.hpp>

#include <memory>

BOOST_AUTO_TEST_SUITE( shared_ptr_tests )

namespace
{
    struct Base
    { };

    struct Derived : public Base
    { };
}

BOOST_AUTO_TEST_CASE( simple )
{
    std::shared_ptr<Derived> p(new Derived());
    BOOST_CHECK( p != NULL );

    std::shared_ptr<Base> pBase(p);
    std::weak_ptr<Base> pWeak(p);
}

BOOST_AUTO_TEST_SUITE_END()
