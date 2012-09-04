#pragma once

struct Precondition
{
    int time;
    int wishtime;

    Precondition()
        : time(-1), wishtime(0)
    { }
	
	virtual ~Precondition()
	{ }

    explicit Precondition(int t)
        : time(t), wishtime(0)
    { }

    bool isFulfilled() const
    {
        return time == 0;
    }

    bool isImpossible() const
    {
        return time == -1;
    }
};
