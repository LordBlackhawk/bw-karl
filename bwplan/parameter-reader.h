#pragma once

#include "bwplan.h"
#include <boost/program_options.hpp>
#include <vector>

using namespace boost;
namespace po = boost::program_options;

struct BWParameterReader
{
	po::options_description general, hidden, all, visible;
	po::positional_options_description p;
	po::variables_map vm;

	bool						showhelp;
	int							oplevel;
	std::string					race;
	std::string					loadfilename;
	std::vector<std::string>	inputops;

	BWParameterReader()
	: general("General options"), hidden("Hidden options"), all("All options"), visible("Allowed options"), showhelp(false)
	{
		general.add_options()
				("help", 		po::bool_switch(&showhelp),								"Show this help message.")
				("optimize,o",	po::value<int>(&oplevel)->default_value(0),				"Optimization level.")
				("race,r",		po::value<std::string>(&race)->default_value("Zerg"),	"Race for start resources.")
				("load,l",		po::value<std::string>(&loadfilename),					"Load build from file name.")
				;

		hidden.add_options()
				("input-op",	po::value< std::vector<string> >(&inputops),		"Input operations.")
				;

		all.add(general).add(hidden);
		visible.add(general);

		p.add("input-op", -1);
	}

	void run(int argc, char *argv[])
	{
		po::store(po::command_line_parser(argc, argv).options(all).positional(p).run(), vm);
		po::notify(vm);
	}

	BWPlan getStartPlan() const
	{
		BWResources res;
		res.set<RMinerals>(50);
		if (race == "Terran") {
			res.set<RTerranWorker>(4);
			res.set<RTerranCommandCenter>(1);
			res.set<RTerranSupply>(2*10);
			res.incLocked<RTerranSupply>(0, 2*4);
		} else if (race == "Protoss") {
			res.set<RProtossWorker>(4);
			res.set<RProtossNexus>(1);
			res.set<RProtossSupply>(2*9);
			res.incLocked<RProtossSupply>(0, 2*4);
		} else if (race == "Zerg") {
			res.set<RZergWorker>(4);
			res.set<RZergHatchery>(1);
			res.set<RZergSupply>(2*9);
			res.incLocked<RZergSupply>(0, 2*4);
			res.set<RLarva>(3);
		} else {
			std::cerr << "Unknown Race: " << race << "\n";
			exit(1);
		}

		BWPlan plan(res, 0);
		if (loadfilename != "")
			plan.loadFromFile(loadfilename);

		for (auto it : inputops) {
			BWOperationIndex index = BWOperationIndex::byUserName(it);
			if (!index.valid()) {
				std::cerr << "Unknown Operation Name: " << it << "\n";
				continue;
			}
			if (!plan.push_back_sr(BWOperation(index)))
				std::cerr << "Unable to add " << index.getName() << "!\n";
		}

		//if (oplevel > 0)
		//	plan.optimize(oplevel);

		return plan;
	}

	template <class Stream>
	friend Stream& operator << (Stream& stream, const BWParameterReader& o)
	{
		stream << o.visible;
	}
};

