#pragma once

#include <algorithm>

namespace VectorHelper
{
	template <class C, class E>
	void remove(C& container, E& element)
	{
		container.erase(std::remove(container.begin(), container.end(), element), container.end());
	}

	template <class C, class F>
	void remove_if(C& container, F f)
	{
		container.erase(std::remove_if(container.begin(), container.end(), f), container.end());
	}
	
	template <class C>
	void clear_and_delete(C& container)
	{
		for (auto it : container)
			delete it;
		container.clear();
	}
	
	template <class C>
	void clear_and_delete_second(C& container)
	{
		for (auto it : container)
			delete it.second;
		container.clear();
	}
}
