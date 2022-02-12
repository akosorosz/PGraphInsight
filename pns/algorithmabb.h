#ifndef ALGORITHMABB_H
#define ALGORITHMABB_H

#include <limits>
#include "algorithmbase.h"
#include "reducedpnsproblemview.h"

namespace PnsTools {

class AlgorithmABB : public AlgorithmBase
{
	struct ABBSolution{
		OperatingUnitSet units;
		MaterialSet materials;
		double objectiveValue;
	};

	// solution
	std::list<ABBSolution> mSolutionStructures;
	std::list<StepOfAlgorithm> mSteps;

	// other options
	unsigned int mMaxSolutionCount;
	EvaluationType mEvaluation;
	bool mUseNeutralExtension;
	unsigned int mMaxParallelProduction;

	int mTotalSolCount;
	int mStepId;
	double mCurrentBest;
	double mGlobalBound;

	void insertSolutionSortedByObjectiveValue(const ABBSolution &solution);

	// last 3 parameters are only used for displaying the steps
	void abbRecursive(const PnsProblem &problem, const MaterialSet &pToBeProduced, const MaterialSet &pAlreadyProduced, const DecisionMapping &pDecisionMap, int parentStepId);
	double getBound(const PnsProblem &problem, const OperatingUnitSet &includedUnits, const OperatingUnitSet &excludedUnits);

	// Different bounding functions
	double getSumOfIncludedWeights(const OperatingUnitSet &includedUnits);

public:
	AlgorithmABB(const PnsProblem &problem, unsigned int maxSolutions, EvaluationType evaluation, unsigned int accelerations=0, unsigned int maxParallelProduction=10000000);
	void run() override;

	const std::list<ABBSolution> &getSolutionStructures() const;
	const std::list<StepOfAlgorithm> &getSteps() const;
};

} // namespace PnsTools

#endif // ALGORITHMABB_H
