#pragma once

struct MicroTaskEnum
{
	enum Type {
		RegionMove,
		ChokepointMove,
		LongMove,

		GatherMinerals,
		GatherGas,

		Build,
		BuildObserver,

		Morph,
		MorphObserver,

		Train,
		TrainObserver,

		Tech,
		TechObserver,

		Upgrade,
		UpgradeObserver,
		
		Scout,

		None = -1
	};
};
