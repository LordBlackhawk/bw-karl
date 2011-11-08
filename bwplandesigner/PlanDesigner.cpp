#define  NO_ASSOCIATIONS
#include "bwplan/bwplan.h"
#include "bwplan/stream-output.h"
#include "bwplan/parameter-reader.h

#include <iostream>

int main(int argc, const char* argv[])
{
  BWParameterReader reader;
  std::string savefilename;
  reader.general.add_options()  ("save,s", po::value<std::string>(&savefilename), "Save build to file name.");
  reader.run(argc, argv);
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
	for (auto it : plan) {
		std::cout << "planed(" << it.time() << "): \t" << outResources(it.getResources());
	}
	std::cout << "\n";

	std::cout << "Plan finished after " << outTime(plan.end().time()) << " frames.\n";
	
  if (savefilename != "")
     plan.saveToFile(savefilename);
	
	return 0;
}
