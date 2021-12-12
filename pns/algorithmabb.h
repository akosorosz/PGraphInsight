#ifndef ALGORITHMABB_H
#define ALGORITHMABB_H

#include <limits>
#include "algorithmbase.h"
#include "reducedpnsproblemview.h"

namespace PnsTools {

class AlgorithmABB : public AlgorithmBase
{
public:
	struct ABBSolution{
		OperatingUnitSet units;
		MaterialSet materials;
		double objectiveValue;
	};

	enum EvaluationType{
		EVAL_NONE,
		EVAL_SUMWEIGHT,
		EVAL_MILP,
		EVAL_MINLP
	};

	// solution
	std::list<ABBSolution> mSolutionStructures;
	std::list<StepOfAlgorithm> mSteps;

	// other options
	unsigned int mMaxSolutionCount;
	EvaluationType mEvaluation;

	int mTotalSolCount;
	int mStepId;
	double mCurrentBest;
	double mGlobalBound;

	void insertSolutionSortedByObjectiveValue(const ABBSolution &solution);

	// last 3 parameters are only used for displaying the steps
	void abbRecursive(const ReducedPnsProblemView &problem, const MaterialSet &toBeProduced, const MaterialSet &alreadyProduced, const DecisionMapping &decisionMap, const OperatingUnitSet &includedUnits, const OperatingUnitSet &excludedUnits, int parentStepId);
	double getBound(const ReducedPnsProblemView &problem, const OperatingUnitSet &includedUnits, const OperatingUnitSet &excludedUnits);

	// Different bounding functions
	double getSumOfIncludedWeights(const OperatingUnitSet &includedUnits);

public:
	AlgorithmABB(const PnsProblem &problem, unsigned int maxSolutions, EvaluationType evaluation);
	void run() override;

	const std::list<ABBSolution> &getSolutionStructures() const;
	const std::list<StepOfAlgorithm> &getSteps() const;
};

} // namespace PnsTools

#endif // ALGORITHMABB_H
