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

template <class P1, class P2>
struct AndPredicate
{
    typedef bool result_type;
    P1 p1;
    P2 p2;
    AndPredicate(const P1& p1_, const P2& p2_)
        : p1(p1_), p2(p2_)
    { }
    template <class ... Args>
    bool operator () (Args... args) const
    {
        if (!p1(args...))
            return false;
        return p2(args...);
    }
};

template <class P1, class P2>
struct OrPredicate
{
    typedef bool result_type;
    P1 p1;
    P2 p2;
    OrPredicate(const P1& p1_, const P2& p2_)
        : p1(p1_), p2(p2_)
    { }
    template <class ... Args>
    bool operator () (Args... args) const
    {
        if (p1(args...))
            return true;
        return p2(args...);
    }
};

template <class P>
struct NotPredicate
{
    typedef bool result_type;
    P p;
    NotPredicate(const P& p_)
        : p(p_)
    { }
    template <class ... Args>
    bool operator () (Args... args) const
    {
        return !p(args...);
    }
};

template <class P1, class P2>
typename enable_if_three_equal<typename P1::result_type, typename P2::result_type, bool, AndPredicate<P1, P2>>::type
operator && (const P1& p1, const P2& p2)
{
    return AndPredicate<P1, P2>(p1, p2);
}

template <class P1, class P2>
typename enable_if_three_equal<typename P1::result_type, typename P2::result_type, bool, OrPredicate<P1, P2>>::type
operator || (const P1& p1, const P2& p2)
{
    return OrPredicate<P1, P2>(p1, p2);
}

template <class P>
typename enable_if_two_equal<typename P::result_type, bool, NotPredicate<P>>::type
operator ! (const P& p)
{
    return NotPredicate<P>(p);
}
