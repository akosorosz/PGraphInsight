#ifndef ALGORITHMSSG_H
#define ALGORITHMSSG_H

#include "algorithmbase.h"
#include "reducedpnsproblemview.h"

namespace PnsTools {

class AlgorithmSSG : public AlgorithmBase
{
	// solution
	std::list<std::pair<OperatingUnitSet,MaterialSet>> mSolutionStructures;
	std::list<StepOfAlgorithm> mSteps;

	int mStepId;

	// last 3 parameters are only used for displaying the steps
	void ssgRecursive(const ReducedPnsProblemView &problem, const MaterialSet &toBeProduced, const MaterialSet &alreadyProduced, const DecisionMapping &decisionMap, const OperatingUnitSet &includedUnits, const OperatingUnitSet &excludedUnits, int parentStepId);

public:
	AlgorithmSSG(const PnsProblem &problem);
	void run() override;

	const std::list<std::pair<OperatingUnitSet,MaterialSet>> &getSolutionStructures() const;
	const std::list<StepOfAlgorithm> &getSteps() const;
};

} // namespace PnsTools

#endif // ALGORITHMSSG_H
