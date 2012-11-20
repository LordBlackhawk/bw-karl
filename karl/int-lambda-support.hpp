#pragma once

#include "enable-if.hpp"

template <class P1, class P2>
struct AddPredicate
{
    typedef int result_type;
    P1 p1;
    P2 p2;
    AddPredicate(const P1& p1_, const P2& p2_)
        : p1(p1_), p2(p2_)
    { }
    template <class ... Args>
    int operator () (Args... args) const
    {
        return p1(args...) + p2(args...);
    }
};

template <class P1, class P2>
struct SubPredicate
{
    typedef int result_type;
    P1 p1;
    P2 p2;
    SubPredicate(const P1& p1_, const P2& p2_)
        : p1(p1_), p2(p2_)
    { }
    template <class ... Args>
    int operator () (Args... args) const
    {
        return p1(args...) - p2(args...);
    }
};

template <class P1, class P2>
struct MulPredicate
{
    typedef int result_type;
    P1 p1;
    P2 p2;
    MulPredicate(const P1& p1_, const P2& p2_)
        : p1(p1_), p2(p2_)
    { }
    template <class ... Args>
    int operator () (Args... args) const
    {
        return p1(args...) * p2(args...);
    }
};

template <class P1, class P2>
struct DivPredicate
{
    typedef int result_type;
    P1 p1;
    P2 p2;
    DivPredicate(const P1& p1_, const P2& p2_)
        : p1(p1_), p2(p2_)
    { }
    template <class ... Args>
    int operator () (Args... args) const
    {
        return p1(args...) / p2(args...);
    }
};

template <class P>
struct MulPredicateConst
{
    typedef int result_type;
    P p;
    int v;
    MulPredicateConst(const P& p_, int v_)
        : p(p_), v(v_)
    { }
    template <class ... Args>
    int operator () (Args... args) const
    {
        return v * p(args...);
    }
};

template <class P>
struct DivPredicateConst
{
    typedef int result_type;
    P p;
    int v;
    DivPredicateConst(const P& p_, int v_)
        : p(p_), v(v_)
    { }
    template <class ... Args>
    int operator () (Args... args) const
    {
        return p(args...) / v;
    }
};

template <class P1, class P2>
typename enable_if_three_equal<typename P1::result_type, typename P2::result_type, int, AddPredicate<P1, P2>>::type
operator + (const P1& p1, const P2& p2)
{
    return AddPredicate<P1, P2>(p1, p2);
}

template <class P1, class P2>
typename enable_if_three_equal<typename P1::result_type, typename P2::result_type, int, SubPredicate<P1, P2>>::type
operator - (const P1& p1, const P2& p2)
{
    return SubPredicate<P1, P2>(p1, p2);
}

template <class P1, class P2>
typename enable_if_three_equal<typename P1::result_type, typename P2::result_type, int, MulPredicate<P1, P2>>::type
operator * (const P1& p1, const P2& p2)
{
    return MulPredicate<P1, P2>(p1, p2);
}

template <class P1, class P2>
typename enable_if_three_equal<typename P1::result_type, typename P2::result_type, int, DivPredicate<P1, P2>>::type
operator / (const P1& p1, const P2& p2)
{
    return DivPredicate<P1, P2>(p1, p2);
}

template <class P>
typename enable_if_two_equal<typename P::result_type, int, MulPredicateConst<P>>::type
operator * (int v, const P& p)
{
    return MulPredicateConst<P>(p, v);
}

template <class P>
typename enable_if_two_equal<typename P::result_type, int, DivPredicateConst<P>>::type
operator * (const P& p, int v)
{
    return DivPredicateConst<P>(p, v);
}
