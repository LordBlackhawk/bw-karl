#include <boost/test/unit_test.hpp>

#include "utils/enum-set.hpp"

namespace
{
    enum class TypeSaveEnum { first, second, third, lastElement };

    std::ostream& operator << (std::ostream& stream, TypeSaveEnum value)
    {
        switch (value) {
            case TypeSaveEnum::first:
                stream << "first";
                break;
            case TypeSaveEnum::second:
                stream << "second";
                break;
            case TypeSaveEnum::third:
                stream << "third";
                break;
            default:
                stream << "[undef value]";
                break;
        }
        return stream;
    }
}

BOOST_AUTO_TEST_SUITE( enum_tests )

BOOST_AUTO_TEST_CASE( basic )
{
    TypeSaveEnum v2 = TypeSaveEnum::first;
    BOOST_CHECK( v2 != TypeSaveEnum::second );

    // Should not compile:
    //TypeSaveEnum v3 = 2;
}

BOOST_AUTO_TEST_CASE( set_test )
{
    EnumSet<TypeSaveEnum> s = { TypeSaveEnum::first, TypeSaveEnum::third };
    BOOST_CHECK(   s.contains(TypeSaveEnum::first) );
    BOOST_CHECK( ! s.contains(TypeSaveEnum::second) );
    BOOST_CHECK(   s.contains(TypeSaveEnum::third) );
    BOOST_CHECK( ! s.empty() );

    EnumSet<TypeSaveEnum> invs = s.inverse();
    BOOST_CHECK( ! invs.contains(TypeSaveEnum::first) );
    BOOST_CHECK(   invs.contains(TypeSaveEnum::second) );
    BOOST_CHECK( ! invs.contains(TypeSaveEnum::third) );
    BOOST_CHECK( ! invs.empty() );

    for (auto it : invs)
        BOOST_CHECK_EQUAL( it, TypeSaveEnum::second );

    EnumSet<TypeSaveEnum> all = EnumSet<TypeSaveEnum>::all();
    BOOST_CHECK(   all.contains(TypeSaveEnum::first) );
    BOOST_CHECK(   all.contains(TypeSaveEnum::second) );
    BOOST_CHECK(   all.contains(TypeSaveEnum::third) );
    BOOST_CHECK( ! all.empty() );

    all.clear(TypeSaveEnum::first);
    BOOST_CHECK( ! all.contains(TypeSaveEnum::first) );
    BOOST_CHECK(   all.contains(TypeSaveEnum::second) );
    BOOST_CHECK(   all.contains(TypeSaveEnum::third) );
    BOOST_CHECK( ! all.empty() );
}

BOOST_AUTO_TEST_CASE( array_test )
{
    EnumArray<int, TypeSaveEnum> arr = { 0, 0, 0 };

    BOOST_CHECK_EQUAL( arr[TypeSaveEnum::first],  0 );

    for (auto& it : arr)
        it = 2;

    arr[TypeSaveEnum::first] = 5;
    arr[TypeSaveEnum::third] = 7;

    BOOST_CHECK_EQUAL( arr[TypeSaveEnum::first],  5 );
    BOOST_CHECK_EQUAL( arr[TypeSaveEnum::second], 2 );
    BOOST_CHECK_EQUAL( arr[TypeSaveEnum::third],  7 );
}

BOOST_AUTO_TEST_SUITE_END()
