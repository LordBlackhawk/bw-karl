// In this file the resource types are defined.

DEF_RESTYPE(Minerals);
DEF_RESTYPE(CommandCenter);
DEF_RESTYPE(Worker);
DEF_RESTYPE(TerranSupply);
DEF_RESTYPE(Depot);

typedef type_list<Minerals, CommandCenter, Worker>		res_list;

DEF_RESGROWTH(Minerals)
{
  const int factor = 1;
  return factor * res.getNotLocked<Worker>();
}
