#pragma once

#include "typelists.h"
#include <array>

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
		return (*this)[indexof<ID, LIST>::value];
	}
	
	template <class ID>
	const T& get() const
	{
		return (*this)[indexof<ID, LIST>::value];
	}
};
