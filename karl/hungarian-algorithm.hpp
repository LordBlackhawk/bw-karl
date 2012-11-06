#pragma once

#include "log.hpp"

#include <vector>
//#include <fstream>

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

		void execute(/*bool check = false*/)
		{
			int nAgents = p.numberOfAgents();
			int nJobs   = p.numberOfJobs();
			if ((nAgents == 0) || (nJobs == 0))
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

            /*
            if (!check)
                return;

            for (int w=0; w<impl.dim; ++w)
				for (int j=0; j<impl.dim; ++j)
					impl.costMatrix(w, j) = p.evaluate(w, j);
                    
            std::fstream stream("test.txt", std::fstream::out);
            for (int w=0; w<impl.dim; ++w) {
				for (int j=0; j<impl.dim; ++j)
                    stream << impl.costMatrix(w, j) << "\t ";
                stream << "\n";
            }
            stream.close();

            for (int w1=0; w1<impl.dim; ++w1) {
                int j1 = impl.matchJobByWorker[w1];
                for (int w2=0; w2<impl.dim; ++w2) {
                    int j2 = impl.matchJobByWorker[w2];

                    double costs = impl.costMatrix(w1, j1) + impl.costMatrix(w2, j2);
                    double costx = impl.costMatrix(w1, j2) + impl.costMatrix(w2, j1);
                    if (costs > costx) {
                        WARNING << "solution of hungarian-algorithm is not optimal!";
                        return;
                    }
                }
            }
            */
		}
};
