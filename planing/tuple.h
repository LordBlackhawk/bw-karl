#pragma once

#include "typelists.h"
#include <array>
#include <cassert>

template <class LIST, class T>
struct Tuple : public std::array<T, size<LIST>::value>
{
	Tuple()
	{ }

	Tuple(const T& value)
	{
		setAll(value);
	}
	
	void setAll(const T& value)
	{
		for (auto& it : *this)
			it = value;
	}

	template <class ID>
	T& get()
	{
		int index = indexof<ID, LIST>::value;
		assert(index >= 0);
		return (*this)[index];
	}
	
	template <class ID>
	const T& get() const
	{
		int index = indexof<ID, LIST>::value;
		assert(index >= 0);
		return (*this)[index];
	}
};
