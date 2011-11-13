#pragma once

#include "bwplan.h"
#include <boost/program_options.hpp>
#include <vector>
#include <string>

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
				("input-op",	po::value< std::vector<std::string> >(&inputops),		"Input operations.")
				;

		p.add("input-op", -1);
	}

	void run(int argc, const char *argv[])
	{
		all.add(general).add(hidden);
		visible.add(general);

		po::store(po::command_line_parser(argc, argv).options(all).positional(p).run(), vm);
		po::notify(vm);
	}

	BWPlan getStartPlan() const
	{
		BWResources res;
		res.set(BWResourceIndex::Minerals, 50);
		if (race == "Terran") {
			res.set(BWResourceIndex::TerranWorker, 4);
			res.set(BWResourceIndex::TerranCommandCenter, 1);
			res.set(BWResourceIndex::TerranSupply, 2*10);
			res.incLocked(BWResourceIndex::TerranSupply, 0, 2*4);
		} else if (race == "Protoss") {
			res.set(BWResourceIndex::ProtossWorker, 4);
			res.set(BWResourceIndex::ProtossNexus, 1);
			res.set(BWResourceIndex::ProtossSupply, 2*9);
			res.incLocked(BWResourceIndex::ProtossSupply, 0, 2*4);
		} else if (race == "Zerg") {
			res.set(BWResourceIndex::ZergWorker, 4);
			res.set(BWResourceIndex::ZergHatchery, 1);
			res.set(BWResourceIndex::ZergSupply, 2*9);
			res.incLocked(BWResourceIndex::ZergSupply, 0, 2*4);
			res.set(BWResourceIndex::Larva, 3);
		} else {
			std::cerr << "Unknown Race: " << race << "\n";
			exit(1);
		}

		BWPlan plan(res, 0);
		if (loadfilename != "")
			plan.loadFromFile(loadfilename.c_str());

		for (auto it : inputops) {
			BWOperationIndex index = BWOperationIndex::byName(it);
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
		return stream;
	}
};

