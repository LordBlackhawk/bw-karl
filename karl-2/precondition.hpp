#pragma once

#include <cassert>

struct Precondition
{
	static const int Max        = 100 * 1000 * 1000;
	static const int Impossible = 2 * Max;
	
    int time;
    int wishtime;

    Precondition()
        : time(Impossible), wishtime(0)
    { }
	
	explicit Precondition(int t)
        : time(t), wishtime(0)
    { }
	
	virtual ~Precondition()
	{ }

    bool isFulfilled() const
    {
		if (this == NULL)
			return true;

        return time == 0;
    }
	
	void setFulfilled()
	{
		assert(this != NULL);
		time = 0;
	}

    bool isImpossible() const
    {
		if (this == NULL)
			return false;

        return time >= Max;
    }
	
	void setImpossible()
	{
		assert(this != NULL);
		time = Impossible;
	}
};
