#include <iostream>

#include "resources.h"
#include "operations.h"
#include "plan.h"

#include "broodwar_resources.h"
#include "broodwar_operations.h"

template <class T>
struct outputName
{
	static void call(int& i)
	{
		std::cout << "Name " << i++ << ": " << ResourceName<T>::name << "\n";
	}
};

template <class T>
struct outputResourcesInternal
{
	static void call(const Resources<res_list>& res, bool& first)
	{
		if (!first)
			std::cout << ", ";
		std::cout << res.get<T>() << " " << ResourceName<T>::name;
		first = false;
	}
};

void outputResources(const Resources<res_list>& res)
{
	bool first = true;
	enumerate<res_list>::call<outputResourcesInternal, const Resources<res_list>&, bool&>(res, first);
	std::cout << "\n";
}

typedef Resources<res_list>				bwRes;
typedef Operation<res_list, op_list>	bwOp;
typedef Plan<res_list, op_list>			bwPlan;

int main()
{
	// StartResources:
	bwRes res;
	res.set<Minerals>(100);
	res.set<CommandCenterTime>(1);
	res.set<Worker>(4);
	
	// Empty Plan:
	bwPlan plan(res);
	
	// Output Plan:
	std::cout << "Empty Plan:\n";
	for (auto it : plan)
	{
		std::cout << it.time() << ": ";
		outputResources(it.getResources());
	}
	std::cout << "\n";

	// Generate a plan:
	while (plan.push_back(bwOp::get<BuildWorker>()))
	{
		std::cout << "Added a BuildWorker-Operation! New endtime: " << plan.endTime() << "\n";
	}
	std::cout << "\n";
	
	// Number of Operations:
	std::cout << "Number of Operations: " << plan.size() << "\n";
	std::cout << "\n";
	
	// Output Plan:
	for (auto it : plan)
	{
		std::cout << it.time() << ": ";
		outputResources(it.getResources());
	}
	std::cout << "\n";

	std::cout << "finished.\n";
	return 0;
}

