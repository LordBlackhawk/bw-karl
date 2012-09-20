#pragma once

#include <algorithm>
#include <set>
#include <vector>

namespace Containers
{
	template <class T>
	void remove(std::vector<T>& container, const T& element)
	{
		container.erase(std::remove(container.begin(), container.end(), element), container.end());
	}

	template <class T>
	void remove(std::set<T>& container, const T& element)
	{
		container.erase(element);
	}
	
	template <class T, class F>
	void remove_if(std::vector<T>& container, F f)
	{
		container.erase(std::remove_if(container.begin(), container.end(), f), container.end());
	}
	
	template <class T, class F>
	void remove_if(std::set<T>& container, F f)
	{
		auto it    = container.begin();
		auto itend = container.end();
		while (it != itend)
			if (f(*it)) {
				it = container.erase(it);
			} else {
				++it;
			}
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
	
	template <class C>
	void fill(C& container, const typename C::value_type& element)
	{
		std::fill(container.begin(), container.end(), element);
	}
}
