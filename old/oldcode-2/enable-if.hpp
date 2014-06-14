#pragma once

template <class A, class B, class T>
struct enable_if_two_equal
{ };

template <class A, class T>
struct enable_if_two_equal<A, A, T>
{
    typedef T type;
};

template <class A, class B, class C, class T>
struct enable_if_three_equal
{ };

template <class A, class T>
struct enable_if_three_equal<A, A, A, T>
{
    typedef T type;
};
