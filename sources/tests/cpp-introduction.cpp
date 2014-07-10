#include <boost/test/unit_test.hpp>

#include <iostream>

#include "utils/timer.hpp"
#include "a.hpp"

#define max(a, b) (((a) > (b)) ? (a) : (b))

void swap(int& x, int& y)
{
    int buf = x;
    x = y;
    y = buf;
}

void sort(int a[], int len)
{
    for (int k=0; k<len; ++k)
        for (int l=0; l<len-1; ++l)
            if (a[l] < a[l+1])
                swap(a[l], a[l+1]);
}

int f()
{
    static auto value = 10;
    return value--;
}

BOOST_AUTO_TEST_SUITE( cpp_introduction )

BOOST_AUTO_TEST_CASE( sort_test )
{
    const int len = 5;
    int a[5] = { 7, 4, 1, 2, max(3, 8) };
                // (((3) > (8)) ? (3) : (8))
    sort(a, len);
            
    for (int k=0; k<len-1; ++k)
        BOOST_CHECK_MESSAGE( a[k] > a[k+1], "k: " << k << "; a[k]: " << a[k] << "; a[k+1]: " << a[k+1] );
}

#undef max

BOOST_AUTO_TEST_CASE( int_pointer_test )
{
    int* ptr = NULL;
    int x = 5;
    ptr = &x;
    *ptr = 10;
    BOOST_CHECK_EQUAL( x, 10 );
}

struct ListItem
{
    ListItem* next;
    int content;
};

void add(ListItem*& anker, int value)
{
    ListItem* newitem = new ListItem;
    newitem->next = anker;
    newitem->content = value;
    anker = newitem;
}

BOOST_AUTO_TEST_CASE( add_pointer_test )
{
    ListItem* anker = NULL;

    add(anker, 10);
    add(anker, 11);

    /*
    ListItem* iterator = anker;
    while (iterator != NULL) {
        std::cout << "value: " << iterator->content << ".\n";
        iterator = iterator->next;
    }
    */
}

BOOST_AUTO_TEST_CASE( pointer_test )
{
    ListItem item;
    item.content = 10;

    ListItem* ptr = &item;
    (*ptr).content = 11;
    ptr->content = 11;

    ListItem* p = new ListItem;
    delete p;
}

/*
BOOST_AUTO_TEST_CASE( test_name )
{
    // type variablenname = initvalue;
    int x = 1;
    bool b = (true || false) && !false;
    double d = ((b) ? 4.0 : 1.0);

    d += 5.0;
    d = d + 5.0;
    b = (d == 0.0) && (d != 1.0);
    x %= 5;
    x = x % 5;

    // b = (a == d); => FEHLER

    if (x == 5) {
        d = 7.0;
    } else if (x == 6) {
        d = 4.0;
    } else {
        d = 6.0;
    }

    switch (x)
    {
        case 5:
        {
            d = 7.0;
            int w = 5;
            break;
        }
        case 6:
            d = 4.0;
            break;
        default:
            d = 6.0;
            break;
    }

    x = 21;
    while (x > 0) {
        int y = --x;
        if (y == 42)
            break;
        int z = x--;
    }

    do {
        x -= 1;
    } while (x > 0);

    const int len = 10;
    //len = 11; => FEHLER
    int a[len] = {0, 1, 3, 4, 5, 6, 7, 8, 9, 10};
    for (int k=0; k<10; ++k) {
        a[k] = k;
    }
    a[11] = 0;
}
*/

BOOST_AUTO_TEST_SUITE_END()









