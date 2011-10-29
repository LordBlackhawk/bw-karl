#pragma once

#include <boost/type_traits/is_same.hpp>

namespace TL {

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
	enum { next  = indexof< C, type_list<T...> >::value };
	enum { value = (boost::is_same<C, F>::value) ? 0 : ((next!=-1) ? next+1 : -1) };
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

template <bool, class T>
struct sublist_helper
{
	typedef type_list < > type;
};

template <class T>
struct sublist_helper<true, T>
{
	typedef type_list < T > type;
};

template <template <class> class PREDICATE, class LIST>
struct sublist
{
	typedef type_list<> type;
};

template <template <class> class PREDICATE, class F, class ...T>
struct sublist< PREDICATE, type_list<F, T...> >
{
	typedef typename combine<
						typename sublist_helper<PREDICATE<F>::value, F>::type,
						typename sublist< PREDICATE, type_list<T...> >::type
					>::type type;
};

template <template <class> class FUNCTION, class LIST>
struct maplist
{
	typedef type_list<> type;
};

template <template <class> class FUNCTION, class F, class ... T>
struct maplist< FUNCTION, type_list<F, T...> >
{
	typedef typename combine<
						type_list< typename FUNCTION<F>::type >,
						typename maplist< FUNCTION, type_list<T...> >::type
					>::type type;
};

template <template <class, class> class FUNC, class LIST>
struct tensorlist
{
	typedef type_list<> type;
};

template <template <class, class> class FUNC, class F, class ... T>
struct tensorlist< FUNC, type_list<F, T...> >
{
	template <class ARG2>
	struct Func
	{
		typedef FUNC< F, ARG2 > type;
	};
	
	template <class ARG1>
	struct InvF
	{
		typedef FUNC< ARG1, F > type;
	};
	
	typedef typename maplist< Func, type_list<T...> >::type  RES1;
	typedef typename maplist< InvF, type_list<T...> >::type  RES2;
	
	typedef typename combine<
				typename combine< RES1, RES2 >::type,
				typename tensorlist< FUNC, type_list<T...> >::type
			>::type type;
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

} // namespace TL