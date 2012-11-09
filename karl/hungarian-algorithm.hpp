#pragma once

//#include "log.hpp"
//#include <cstdlib>
#include <vector>

struct HungarianAlgorithmImpl
{
    typedef int          ctype;
    ctype infty() const;

	int                     rows;
	int						cols;
	int						dim;
	std::vector<ctype>		costMatrixElements;
	std::vector<ctype>		labelByWorker;
	std::vector<ctype> 	    labelByJob;
	std::vector<int>		minSlackWorkerByJob;
	std::vector<ctype>		minSlackValueByJob;
	std::vector<int>		matchJobByWorker;
	std::vector<int>		matchWorkerByJob;
	std::vector<int>		parentWorkerByCommittedJob;
	std::vector<bool>		committedWorkers;

	ctype& costMatrix(int w, int j)
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
	void updateLabeling(ctype slack);
    bool isValid();
    void writeMatrixToFile(const char* filename);
    void writeAssignmentToFile(const char* filename);
};

template <class ProblemType>
class HungarianAlgorithm
{
	private:
		ProblemType& 			p;
		HungarianAlgorithmImpl	impl;

        void initMatrix()
        {
            int nAgents = p.numberOfAgents();
			int nJobs   = p.numberOfJobs();
			impl.prepare(nAgents, nJobs);
            fillMatrix();
        }

        void fillMatrix()
        {
			for (int w=0; w<impl.dim; ++w)
				for (int j=0; j<impl.dim; ++j)
					impl.costMatrix(w, j) = p.evaluate(w, j);
        }

	public:
		HungarianAlgorithm(ProblemType& p_)
			: p(p_)
		{ }

		void execute()
		{
			initMatrix();
			impl.execute();

            /*
            if (!isValid()) {
                writeMatrixToFile("matrix.txt");
                writeAssignmentToFile("assignment.txt");
                WARNING << "Hungarian algorithm returned wrong solution! (assignment.txt, matrix.txt created.)";
                exit(99);
            }
            */

			for (int w=0; w<impl.dim; ++w) {
				int j = impl.matchJobByWorker[w];
				p.assign(w, j);
			}
        }

        bool isValid()
        {
            fillMatrix();
            return impl.isValid();
        }

        void writeMatrixToFile(const char* filename)
        {
            impl.writeMatrixToFile(filename);
		}

        void writeAssignmentToFile(const char* filename)
        {
            impl.writeAssignmentToFile(filename);
        }
};
