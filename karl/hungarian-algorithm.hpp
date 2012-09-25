#pragma once

#include <vector>

struct HungarianAlgorithmImpl
{
	int 					rows;
	int						cols;
	int						dim;
	std::vector<double>		costMatrixElements;
	std::vector<double>		labelByWorker;
	std::vector<double> 	labelByJob;
	std::vector<int>		minSlackWorkerByJob;
	std::vector<double>		minSlackValueByJob;
	std::vector<int>		matchJobByWorker;
	std::vector<int>		matchWorkerByJob;
	std::vector<int>		parentWorkerByCommittedJob;
	std::vector<bool>		committedWorkers;
	
	double& costMatrix(int w, int j)
	{
		return costMatrixElements[w+dim*j];
	}
	
	void prepare(int r, int c);
	void execute();
	void computeInitialFeasibleSolution();
	void executePhase();
	int fetchUnmatchedWorker();
	void greedyMatch();
	void initializePhase(int w);
	void match(int w, int j);
	void reduce();
	void updateLabeling(double slack);
};

template <class ProblemType>
class HungarianAlgorithm
{	
	private:
		ProblemType& 			p;
		HungarianAlgorithmImpl	impl;
	
	public:
		HungarianAlgorithm(ProblemType& p)
			: p(p)
		{ }
		
		void execute()
		{
			int nAgents = p.numberOfAgents();
			int nJobs   = p.numberOfJobs();
			if ((nAgents == 0) && (nJobs == 0))
				return;
			
			impl.prepare(nAgents, nJobs);
			
			for (int w=0; w<impl.dim; ++w)
				for (int j=0; j<impl.dim; ++j)
					impl.costMatrix(w, j) = p.evaluate(w, j);
			
			impl.execute();
			
			for (int w=0; w<impl.dim; ++w) {
				int j = impl.matchJobByWorker[w];
				p.assign(w, j);
			}
		}
};
