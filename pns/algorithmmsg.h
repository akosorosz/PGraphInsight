#ifndef ALGORITHMMSG_H
#define ALGORITHMMSG_H

#include "pnsproblem.h"
#include "algorithmbase.h"
#include <list>

namespace PnsTools {

class AlgorithmMSG : public AlgorithmBase
{
	// solution
	OperatingUnitSet mUnitsInMaximalStructure;
	MaterialSet mMaterialsInMaximalStructure;
	std::list<StepOfAlgorithm> mSteps;

	// other options
	OperatingUnitSet mOnlyConsiderTheseUnits;
	OperatingUnitSet mExcludeTheseUnits;
public:
	AlgorithmMSG(const PnsProblem &problem);
	void run() override;
	void onlyConsiderTheseUnits(const OperatingUnitSet &units);
	void excludeTheseUnits(const OperatingUnitSet &units);

	const OperatingUnitSet &getUnitsOfMaximalStructure() const;
	MaterialSet getMaterialsOfMaximalStructure() const;
	const std::list<StepOfAlgorithm> &getSteps() const;
};

} // namespace PnsTools

#endif // ALGORITHMMSG_H
