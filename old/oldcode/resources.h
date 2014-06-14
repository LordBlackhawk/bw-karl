#ifndef RESOURCES_h
#define RESOURCES_h

class Task;

class Resources
{
  public:
    int minerals;
    int gas;
    int supply;
    
    Task* owner;
    
  public:
    Resources(Task* o=NULL, int m=0, int g=0, int s=0)
		: minerals(m), gas(g), supply(s), owner(o)
    { }
    
    Resources& operator -= (const Resources& res)
    {
      minerals -= res.minerals;
      gas      -= res.gas;
      supply   -= res.supply;
      return *this;
    }

	Resources& operator += (const Resources& res)
    {
      minerals += res.minerals;
      gas      += res.gas;
      supply   += res.supply;
      return *this;
    }
    
    bool operator <= (const Resources& res)
    {
      return   ((minerals <= res.minerals) || ((minerals == 0) && (res.minerals < 0)))
	        && ((gas      <= res.gas)      || ((gas == 0) && (res.gas      < 0)))
	        && ((supply   <= res.supply)   || ((supply == 0) && (res.supply   < 0)));
    }
};
  

#endif