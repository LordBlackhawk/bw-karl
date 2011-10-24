#ifndef PRIORITIES_h
#define PRIORITIES_h

class Priority
{
public:
	enum {
		BaseManager     = 1000,

		GatherMinerals	= 100,
		GatherGas		= 300,
		BuildOverlord	= 700,
		ExtractorTrick  = 600,
		BuildHatch		= 500,
		BuildPool		= 400,
		BuildThirdBase  = 200,

		Lair			= 200,
		ZergSpeed       = 400,
		EarlyZergling   = 420,
		FirstZerglings	= 420,
		BuildColony		= 400,

		ScoutOverlord	= 200
	};
};

#endif