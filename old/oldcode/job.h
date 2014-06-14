#ifndef JOB_h
#define JOB_h

class Job
{
public:
	enum JobType
	{
		Reset,				// No Parameter!
		Waiting,			// No Parameter!
		GoTo,				// position
		GatherMinerals,		// target
		GatherGas,			// target
		Build,				// unittype, tileposition, res, target (falls schon angefangen, dann ist das das gebäude!)
		BuildWaiting,		// only Zerg!
		TrainUnit,			// unittype
		Morph,				// unittype
		ScoutPosition,		// position
		AttackPosition,		// position
		AttackNoDamage,		// target
		Research,			// techtype
		Upgrade				// upgradetype
	};

	JobType			type;
	Unit* 			target;
	Position 		position;
	TilePosition	tileposition;
	Resources*		res;
	UnitType		unittype;
	TechType		techtype;
	UpgradeType		upgradetype;

public:
	Job()
		: type(Waiting), target(NULL), res(NULL)
	{ }

	Job(JobType jt)
		: type(jt), target(NULL), res(NULL)
	{ }

	Job(JobType jt, Unit* t)
		: type(jt), target(t), res(NULL)
	{ }

	Job(JobType jt, TilePosition tp, UnitType ut, Resources* r) 
		: type(jt), target(NULL), tileposition(tp), res(r), unittype(ut)
	{ }

	Job(JobType jt, UnitType ut, Resources* r) 
		: type(jt), target(NULL), res(r), unittype(ut)
	{ }

	Job(JobType jt, Position p)
		: type(jt), target(NULL), position(p), res(NULL)
	{ }

	Job(JobType jt, TechType tt, Resources* r)
		: type(jt), target(NULL), res(r), techtype(tt)
	{ }

	Job(JobType jt, UpgradeType ut, Resources* r)
		: type(jt), target(NULL), res(r), upgradetype(ut)
	{ }

	static Job wait()
	{
		return Job(Waiting);
	}

	static Job gatherMinerals(Unit* t)
	{
		return Job(GatherMinerals, t);
	}

	static Job gatherGas(Unit* t)
	{
		return Job(GatherGas, t);
	}

	static Job build(TilePosition tp, UnitType ut, Resources* r)
	{
		return Job(Build, tp, ut, r);
	}

	static Job morph(UnitType ut, Resources* r)
	{
		return Job(Morph, ut, r);
	}

	static Job goTo(Position p)
	{
		return Job(GoTo, p);
	}

	static Job attackPosition(Position p)
	{
		return Job(AttackPosition, p);
	}

	static Job scoutPosition(Position p)
	{
		return Job(ScoutPosition, p);
	}

	static Job attackNoDamage(Unit* t)
	{
		return Job(AttackNoDamage, t);
	}

	static Job research(TechType tt, Resources* r)
	{
		return Job(Research, tt, r);
	}

	static Job upgrade(UpgradeType ut, Resources* r)
	{
		return Job(Upgrade, ut, r);
	}

public:
	Job& operator = (JobType jt)
	{
		type = jt;
		return *this;
	}

	bool operator == (JobType jt) const
	{
		return (type == jt);
	}

	bool operator != (JobType jt) const
	{
		return (type != jt);
	}
};

#endif