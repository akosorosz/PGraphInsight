#include "algorithmbase.h"

namespace PnsTools {

AlgorithmBase::AlgorithmBase(const PnsProblem &problem):
	mProblem(problem),
	mOnlyConsiderTheseUnits(problem.operatingUnits())
{
}

void AlgorithmBase::saveSteps(bool save)
{
	mSaveSteps=save;
}

void AlgorithmBase::onlyConsiderTheseUnits(const OperatingUnitSet &units)
{
	mOnlyConsiderTheseUnits=units;
}

void AlgorithmBase::excludeTheseUnits(const OperatingUnitSet &units)
{
	mExcludeTheseUnits=units;
}

} // namespace PnsTools
