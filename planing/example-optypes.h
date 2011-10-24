// In this file the operation types are defined.

BEGIN_DEF_OPTYPE(BuildWorker)
  	 Consums<50, Minerals>,
    Consums<1, TerranSupply>,
		Locks<1, CommandCenter>,
		Duration<50>,
    Unlocks<1, CommandCenter>,
		Prods<1, Worker>
END_DEF_OPTYPE

BEGIN_DEF_OPTYPE(BuildDepot)
    Locks<1, Worker>,
    Duration<10>,
    Consums<100, Minerals>,
    Duration<100>,
    Unlocks<1, Worker>,
    Prods<1, Depot>,
    Prods<8, TerranSupply>
END_DEF_OPTYPE

DEF_OPDETAILS(BuildDepot, BWAPI::TilePosition)

DEF_OPDYNDURATION(BuildDepot, 10)
{
  return /* Entfernung in Berechnung einbeziehen! */ 11;
}

typedef type_list<BuildWorker, BuildDepot>		op_list;