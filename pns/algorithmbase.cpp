#include "algorithmbase.h"

namespace PnsTools {

AlgorithmBase::AlgorithmBase(const PnsProblem &problem):
	mProblem(problem)
{
}

void AlgorithmBase::saveSteps(bool save)
{
	mSaveSteps=save;
}

} // namespace PnsTools
