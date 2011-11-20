#pragma once

#include "micro-task.h"
#include <set>

template <class T>
void settick(std::set<T>& items)
{
	auto it = items.begin();
	while (it != items.end())
		if (it->tick() != TaskStatus::running)
			items.erase(it);
		else
			++it;
}

template <class T>
void settickptr(std::set<T>& items)
{
	auto it = items.begin();
	while (it != items.end())
		if ((*it)->tick() != TaskStatus::running)
			items.erase(it);
		else
			++it;
}