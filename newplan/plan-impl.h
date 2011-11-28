#pragma once

#include "plan.h"
#include "opt-functions.h"
#include "fallbackbehaviour.h"
#include "add-linear-correction.h"

#include "utils/debug.h"
#include "utils/fileutils.h"

#include <boost/regex.hpp>

FallbackBehaviourType::type PlanContainer::push_back_sr(const Operation& op)
{
	DefaultFallbackBehaviour dfbb;
	SimpleFallbackBehaviour<DefaultFallbackBehaviour> sfbb(dfbb);
	return push_back(op, sfbb);
}

FallbackBehaviourType::type PlanContainer::push_back_df(const Operation& op)
{
	DefaultFallbackBehaviour dfbb;
	return push_back(op, dfbb);
}

bool PlanContainer::rebase_sr(TimeType timeinc, const Resources& newres)
{
	DefaultFallbackBehaviour dfbb;
	SimpleFallbackBehaviour<DefaultFallbackBehaviour> sfbb(dfbb);
	return rebase(timeinc, newres, sfbb);
}

bool PlanContainer::rebase_df(TimeType timeinc, const Resources& newres)
{
	DefaultFallbackBehaviour dfbb;
	return rebase(timeinc, newres, dfbb);
}

int PlanContainer::optimizeEndTime(int max_counter)
{
	OptimizeEndTimeFunction func;
	return optimizeLocalMulti(func, max_counter);
}

bool PlanContainer::loadFromFile(const char* filename)
{
	std::string content;
	if (!readFileToString(filename, content))
		return false;
	
	static boost::regex expression("^[[:space:]]*(\\*([[:digit:]]*))?[[:space:]]*([[:word:]]+)$");
	std::string::const_iterator start = content.begin(), end = content.end();
	boost::match_results<std::string::const_iterator> what;
	boost::match_flag_type flags = boost::match_default;
	while ((start != end) && regex_search(start, end, what, expression, flags))
	{
		start = what[0].second;
		// what[0] whole string.
		// what[2] Anzahl.
		// what[3] Operationsname.
		OperationIndex index = OperationIndex::byName(what.str(3));
		if (!index.valid())
			continue;
		int count = (what.str(2) != "") ? atoi(what.str(2).c_str()) : 1;
		for (int k=0; k<count; ++k)
			push_back_df(Operation(index));
	}
	
	return true;
}

bool PlanContainer::saveToFile(const char* filename) const
{
	FILE* file = fopen(filename, "w");
	if (file == NULL)
		return false;
	for (auto it : scheduled_operations)
		fprintf(file, "%s\n", it.getName().c_str());
	fclose(file);
	return true;
}