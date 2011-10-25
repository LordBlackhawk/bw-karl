// In this file the operation types are defined.

BEGIN_DEF_OPTYPE(BuildWorker)
	Consums<50, Minerals>,
    Consums<1, TerranSupply>,
	Locks<1, CommandCenter>,
		CheckPoint<DoBuildWorker>,
	Duration<50>,
    Unlocks<1, CommandCenter>,
	Prods<1, Worker>
END_DEF_OPTYPE

BEGIN_DEF_OPTYPE(BuildDepot)
    Locks<1, Worker>,
    Duration<10>,
    Consums<100, Minerals>,
		CheckPoint<DoBuildDepot>,
    Duration<100>,
    Unlocks<1, Worker>,
    Prods<1, Depot>,
    Prods<8, TerranSupply>
END_DEF_OPTYPE

BEGIN_DEF_OPTYPE(ExtractorTrick)
	Consums<1, Worker>,
	Consums<50, Minerals>,
	Prods<1, ZergSupply>,
		CheckPoint<DoBuildExtractor>,
	Consums<1, ZergSupply>,
	Consums<1, Larva>,
	Consums<50, Minerals>,
		CheckPoint<DoBuildWorker>,
	Consums<1, ZergSupply>
	Prods<1, Worker>,
	Prods<48, Minerals>,
		CheckPoint<DoCancleExtractor>,
	Duration<100>,
	Prods<1, Worker>
END_DEF_OPTYPE

DEF_OPDETAILS(BuildDepot, int)

DEF_OPDYNDURATION(BuildDepot, 10)
{
  return /* Entfernung in Berechnung einbeziehen! */ defaultValue + details + 1;
}

typedef type_list<BuildWorker, BuildDepot>		op_list;