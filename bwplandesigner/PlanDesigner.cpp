#define  NO_ASSOCIATIONS
#include "newplan/bwplan.h"
#include "newplan/nocheckpoints.h"
#include "newplan/stream-output.h"
#include "newplan/parameter-reader.h"

#include <iostream>

int main(int argc, const char* argv[])
{
	
	BWParameterReader reader;
	std::string savefilename;
	reader.general.add_options()  ("save,s", po::value<std::string>(&savefilename), "Save build to file name.");
	try {
		reader.run(argc, argv);
	} catch (std::exception& e) {
		std::cerr << "Error occurred while parsing parameter: " << e.what();
		return 1;
	}
	if (reader.showhelp) {
		std::cerr << "Use: PlanDesigner [Options...] [Operations...]\n" << reader;
		return 1;
	}

	std::cout << "Parsing arguments...\n";
	BWPlan plan = reader.getStartPlan();
	std::cout << "\n";

	std::cout << "Planed Operations:\n";
	for (auto it : plan.scheduledOperations())
		std::cout << "planed(" << it.scheduledTime() << "): " << it.getName() << "\n";
	std::cout << "\n";

	std::cout << "Planed Resources:\n";
	for (auto it : plan)
		std::cout << "planed(" << it.time() << "): \t" << outResources(it.getResources());
	std::cout << "\n";
	
	std::cout << "Corrections:\n";
	for (auto it : plan.getCorrections())
		std::cout << "correction: " << it.interval << ", value = " << it.value << "\n";
	std::cout << "\n";

	std::cout << "Plan finished after " << outTime(plan.end().time()) << " frames.\n";
	
	if (savefilename != "")
		plan.saveToFile(savefilename.c_str());
	
	return 0;
}
