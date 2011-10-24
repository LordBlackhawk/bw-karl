#pragma once

#include <boost/type_traits/is_same.hpp>

struct empty_type
{ };

template <class ... T>
struct type_list
{ };

template <class LIST>
struct size
{ };

template <class ... T>
struct size< type_list<T...> >
{
	enum { value = sizeof...(T) };
};

template <class LIST>
struct pop_front
{ };

template <class F, class ... T>
struct pop_front< type_list<F, T...> >
{
	typedef type_list<T...> type;
};

template <class LIST>
struct front
{ };

template <class F, class ... T>
struct front< type_list<F, T...> >
{
	typedef F type;
};

template <>
struct front< type_list<> >
{
	typedef empty_type type;
};

template <class C, class LIST>
struct indexof
{ };

template <class C, class F, class ... T>
struct indexof<C, type_list<F, T...> >
{
	enum { value = (boost::is_same<C, F>::value) ? 0 : (indexof< C, type_list<T...> >::value + 1) };
};

template <class C>
struct indexof<C, type_list<> >
{
	enum { value = -1 };
};

template <int i, class LIST>
struct byindex
{ };

template <int i, class F, class ... T>
struct byindex< i, type_list<F, T...> >
{
	typedef typename byindex< i-1, type_list<T...> >::type type;
};

template <class F, class ... T>
struct byindex< 0, type_list<F, T...> >
{
	typedef F type;
};

template <class L1, class L2>
struct combine
{ };

template <class ... T, class ... U>
struct combine< type_list<T...>, type_list<U...> >
{
	typedef type_list<T..., U...> type;
};

template <class LIST>
struct enumerate
{
	template <template<class> class ENUMERATOR, class ... ARGS>
	static void call(ARGS... /*args*/)
	{ }
};

template <class F, class ... T>
struct enumerate< type_list<F, T...> >
{
	template <template<class> class ENUMERATOR, class ... ARGS>
	static void call(ARGS... args)
	{
		ENUMERATOR<F>::call(args...);
		enumerate< type_list<T...> >::template call<ENUMERATOR, ARGS...>(args...);
	}
};

template <class LIST>
struct dispatch
{
	template <template<class> class DISPATCHER, class ... ARGS>
	static void call(int /*index*/, ARGS... /*args*/)
	{ }
};

template <class F, class ... T>
struct dispatch< type_list<F, T...> >
{
	template <template<class> class DISPATCHER, class ... ARGS>
	static void call(int index, ARGS... args)
	{
		if (index == 0)
			DISPATCHER<F>::call(args...);
		dispatch< type_list<T...> >::template call<DISPATCHER, ARGS...>(index-1, args...);
	}
};
