#include "hungarian-algorithm.hpp"
#include "container-helper.hpp"
#include <limits>
#include <fstream>

HungarianAlgorithmImpl::ctype HungarianAlgorithmImpl::infty() const
{
    return std::numeric_limits<ctype>::max();
}

void HungarianAlgorithmImpl::prepare(int r, int c)
{
	rows = r;
	cols = c;
	dim  = std::max(rows, cols);
    if (dim == 0)
        return;

	costMatrixElements.resize(dim*dim);
	labelByWorker.resize(dim);
	labelByJob.resize(dim);
	minSlackWorkerByJob.resize(dim);
	minSlackValueByJob.resize(dim);
	committedWorkers.resize(dim);
	parentWorkerByCommittedJob.resize(dim);
	matchJobByWorker.resize(dim);
	matchWorkerByJob.resize(dim);

    Containers::fill(labelByWorker, 0);
	Containers::fill(matchJobByWorker, -1);
	Containers::fill(matchWorkerByJob, -1);
}

void HungarianAlgorithmImpl::execute()
{
	/*
	* Heuristics to improve performance: Reduce rows and columns by their
	* smallest element, compute an initial non-zero dual feasible solution
	* and create a greedy matching from workers to jobs of the cost matrix.
	*/
    
    if (dim == 0)
        return;

	reduce();
	computeInitialFeasibleSolution();
	greedyMatch();

	int w = fetchUnmatchedWorker();
	while (w < dim) {
		initializePhase(w);
		executePhase();
		w = fetchUnmatchedWorker();
	}
}

/**
* Compute an initial feasible solution by assigning zero labels to the
* workers and by assigning to each job a label equal to the minimum cost
* among its incident edges.
*/
void HungarianAlgorithmImpl::computeInitialFeasibleSolution()
{
	for (int j=0; j<dim; ++j)
		labelByJob[j] = infty();

	for (int w=0; w<dim; ++w)
		for (int j=0; j<dim; ++j)
			if (costMatrix(w, j) < labelByJob[j])
				labelByJob[j] = costMatrix(w, j);
}

/**
* Execute a single phase of the algorithm. A phase of the Hungarian
* algorithm consists of building a set of committed workers and a set of
* committed jobs from a root unmatched worker by following alternating
* unmatched/matched zero-slack edges. If an unmatched job is encountered,
* then an augmenting path has been found and the matching is grown. If the
* connected zero-slack edges have been exhausted, the labels of committed
* workers are increased by the minimum slack among committed workers and
* non-committed jobs to create more zero-slack edges (the labels of
* committed jobs are simultaneously decreased by the same amount in order
* to maintain a feasible labeling).
* <p>
*
* The runtime of a single phase of the algorithm is O(n^2), where n is the
* dimension of the internal square cost matrix, since each edge is visited
* at most once and since increasing the labeling is accomplished in time
* O(n) by maintaining the minimum slack values among non-committed jobs.
* When a phase completes, the matching will have increased in size.
*/
void HungarianAlgorithmImpl::executePhase()
{
	while (true) {
		int minSlackWorker = -1, minSlackJob = -1;
		ctype minSlackValue = infty();
		for (int j = 0; j < dim; j++) {
			if (parentWorkerByCommittedJob[j] == -1) {
				if (minSlackValueByJob[j] < minSlackValue) {
					minSlackValue = minSlackValueByJob[j];
					minSlackWorker = minSlackWorkerByJob[j];
					minSlackJob = j;
				}
			}
		}
		if (minSlackValue > 0)
			updateLabeling(minSlackValue);
		parentWorkerByCommittedJob[minSlackJob] = minSlackWorker;
		if (matchWorkerByJob[minSlackJob] == -1) {
			/*
			* An augmenting path has been found.
			*/
			int committedJob = minSlackJob;
			int parentWorker = parentWorkerByCommittedJob[committedJob];
			while (true) {
				int temp = matchJobByWorker[parentWorker];
				match(parentWorker, committedJob);
				committedJob = temp;
				if (committedJob == -1)
					break;
				parentWorker = parentWorkerByCommittedJob[committedJob];
			}
			return;
		} else {
			/*
			* Update slack values since we increased the size of the
			* committed workers set.
			*/
			int worker = matchWorkerByJob[minSlackJob];
			committedWorkers[worker] = true;
			for (int j = 0; j < dim; j++) {
				if (parentWorkerByCommittedJob[j] == -1) {
					ctype slack = costMatrix(worker, j) - labelByWorker[worker] - labelByJob[j];
					if (minSlackValueByJob[j] > slack) {
						minSlackValueByJob[j] = slack;
						minSlackWorkerByJob[j] = worker;
					}
				}
			}
		}
	}
}

/**
* @return the first unmatched worker or {@link #dim} if none.
*/
int HungarianAlgorithmImpl::fetchUnmatchedWorker()
{
	int w=0;
	for (; w<dim; ++w)
		if (matchJobByWorker[w] == -1)
			break;
	return w;
}

/**
* Find a valid matching by greedily selecting among zero-cost matchings.
* This is a heuristic to jump-start the augmentation algorithm.
*/
void HungarianAlgorithmImpl::greedyMatch()
{
	for (int w=0; w<dim; ++w)
		for (int j=0; j<dim; ++j)
	{
		if (   (matchJobByWorker[w] == -1)
			&& (matchWorkerByJob[j] == -1)
			&& (costMatrix(w, j) - labelByWorker[w] - labelByJob[j] == 0))
		{
			match(w, j);
		}
	}
}

/**
* Initialize the next phase of the algorithm by clearing the committed
* workers and jobs sets and by initializing the slack arrays to the values
* corresponding to the specified root worker.
*
* @param w
* the worker at which to root the next phase.
*/
void HungarianAlgorithmImpl::initializePhase(int w)
{
	Containers::fill(committedWorkers, false);
	Containers::fill(parentWorkerByCommittedJob, -1);
	committedWorkers[w] = true;
	for (int j=0; j<dim; ++j) {
		minSlackValueByJob[j] = costMatrix(w, j) - labelByWorker[w] - labelByJob[j];
		minSlackWorkerByJob[j] = w;
	}
}

/**
* Helper method to record a matching between worker w and job j.
*/
void HungarianAlgorithmImpl::match(int w, int j)
{
	matchJobByWorker[w] = j;
	matchWorkerByJob[j] = w;
}

/**
* Reduce the cost matrix by subtracting the smallest element of each row
* from all elements of the row as well as the smallest element of each
* column from all elements of the column. Note that an optimal assignment
* for a reduced cost matrix is optimal for the original cost matrix.
*/
void HungarianAlgorithmImpl::reduce()
{
	for (int w=0; w<dim; ++w) {
		ctype min = infty();
		for (int j=0; j<dim; ++j)
			if (costMatrix(w, j) < min)
				min = costMatrix(w, j);

		for (int j=0; j<dim; ++j)
			costMatrix(w, j) -= min;
	}

	for (int j=0; j<dim; ++j) {
		ctype min = infty();
		for (int w=0; w<dim; ++w)
			if (costMatrix(w, j) < min)
				min = costMatrix(w, j);

		for (int w=0; w<dim; ++w)
			costMatrix(w, j) -= min;
	}
}

/**
* Update labels with the specified slack by adding the slack value for
* committed workers and by subtracting the slack value for committed jobs.
* In addition, update the minimum slack values appropriately.
*/
void HungarianAlgorithmImpl::updateLabeling(ctype slack)
{
	for (int w=0; w<dim; ++w)
		if (committedWorkers[w])
			labelByWorker[w] += slack;

	for (int j=0; j<dim; ++j)
		if (parentWorkerByCommittedJob[j] != -1) {
			labelByJob[j] -= slack;
		} else {
			minSlackValueByJob[j] -= slack;
		}
}

bool HungarianAlgorithmImpl::isValid()
{
    for (int w1=0; w1<dim; ++w1) {
        int j1 = matchJobByWorker[w1];
        for (int w2=0; w2<dim; ++w2) {
            int j2 = matchJobByWorker[w2];

            ctype costs = costMatrix(w1, j1) + costMatrix(w2, j2);
            ctype costx = costMatrix(w1, j2) + costMatrix(w2, j1);
            if (costs > costx)
                return false;
        }
    }
    return true;
}

void HungarianAlgorithmImpl::writeMatrixToFile(const char* filename)
{
    std::fstream stream(filename, std::fstream::out);
    for (int w=0; w<dim; ++w) {
        for (int j=0; j<dim; ++j)
            stream << costMatrix(w, j) << ", ";
        stream << "\n";
    }
    stream.close();
}

void HungarianAlgorithmImpl::writeAssignmentToFile(const char* filename)
{
    std::fstream stream(filename, std::fstream::out);
    for (int w=0; w<dim; ++w)
        stream << w << "\t " << matchJobByWorker[w] << "\n";
    stream.close();
}
