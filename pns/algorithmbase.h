#ifndef ALGORITHMBASE_H
#define ALGORITHMBASE_H

#include "pnsproblem.h"

namespace PnsTools {

struct StepOfAlgorithm
{
	int stepId;
	int parentStepId;
	OperatingUnitSet includedUnits;
	OperatingUnitSet excludedUnits;
	double localBound;
	std::string comment;

	StepOfAlgorithm():
		stepId(-1),parentStepId(-1),localBound(0.0)
	{}

	StepOfAlgorithm(int id, int parent, const OperatingUnitSet &included, const OperatingUnitSet &excluded,
					double bound, const std::string &comment):
		stepId(id),parentStepId(parent),includedUnits(included),excludedUnits(excluded),localBound(bound),comment(comment)
	{}
};

class AlgorithmBase
{
protected:
	const PnsProblem &mProblem;
	bool mSaveSteps=true;

	// other options
	OperatingUnitSet mOnlyConsiderTheseUnits;
	OperatingUnitSet mExcludeTheseUnits;
public:
	AlgorithmBase(const PnsProblem &problem);
	virtual void run() = 0;
	void saveSteps(bool save);
	void onlyConsiderTheseUnits(const OperatingUnitSet &units);
	void excludeTheseUnits(const OperatingUnitSet &units);
};

} // namespace PnsTools

#endif // ALGORITHMBASE_H
