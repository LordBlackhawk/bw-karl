#include "hungarian-algorithm.hpp"
#include "timer.hpp"
#include "utils/debug-clog.h"
#include "utils/debug-2.h"
#include <iostream>
#include <cstdlib>
#include <time.h>

struct ProblemExample
{
	ProblemExample()
	{
		srand(time(NULL));
	}
	
	double evaluate(int idAgent, int idJob)
	{
		/*
		const double table[]
			= {	7.0, 4.0, 3.0, 2.5,
		        6.0, 8.0, 5.0, 8.7,
				9.0, 6.0, 4.0, 4.5,
                4.5, 8.3, 8.1, 8.5  };
		
		return table[numberOfJobs() * idAgent + idJob];
		*/
		return rand() % 20;
	}
	
	void assign(int idAgent, int idJob)
	{
		//std::cout << "Agent " << idAgent << " assigned to Job " << idJob << "\n";
	}
	
	int numberOfAgents()
	{
		return 100;
	}
	
	int numberOfJobs()
	{
		return 100;
	}
};

int main()
{
	std::cout << "Running test...\n";

	timerInit();	
	ProblemExample p;
	HungarianAlgorithm<ProblemExample> solver(p);
	
	timerStart();
	for (int k=0; k<10; ++k)
		solver.execute();
	timerEnd();
	
	std::cout << "time elapsed: " << timerAverage()/10.0 << " ns\n";
	std::cout << "Test finished.\n";
	return 0;
}