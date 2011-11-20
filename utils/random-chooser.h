#pragma once

#include <set>

template <class T>
T getRandomSomething(const std::set<T>& list)
{
	if (list.size() <= 0)
		return NULL;

	int index = rand() % list.size();
	typename std::set<T>::const_iterator it=list.begin();
	while (index > 0)
	{
		--index;
		++it;
	}
	return *it;
}
