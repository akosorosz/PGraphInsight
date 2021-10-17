#ifndef REDUCEDPNSPROBLEMVIEW_H
#define REDUCEDPNSPROBLEMVIEW_H

#include "pnsproblem.h"

namespace PnsTools {

class ReducedPnsProblemView
{
	const PnsProblem &mProblem;
	OperatingUnitSet mBaseUnitSet;
public:
	ReducedPnsProblemView(const PnsProblem &problem);
	bool isEmpty() const;
	const MaterialSet &materials() const;
	const MaterialSet &rawMaterials() const;
	const MaterialSet &products() const;
	const MaterialSet &intermediates() const;
	const OperatingUnitSet &operatingUnits() const;
	OperatingUnitSet unitsProducing(const Material &material) const;
	OperatingUnitSet unitsConsuming(const Material &material) const;
	MaterialSet materialsProducedBy(const OperatingUnit &unit) const;
	MaterialSet materialsConsumedBy(const OperatingUnit &unit) const;
	OperatingUnitSet unitsProducingAnyOf(const MaterialSet &materials) const;
	OperatingUnitSet unitsConsumingAnyOf(const MaterialSet &materials) const;
	MaterialSet materialsProducedByAnyOf(const OperatingUnitSet &units) const;
	MaterialSet materialsConsumedByAnyOf(const OperatingUnitSet &units) const;

	void setBaseUnitSet(const OperatingUnitSet &units);
};

} // namespace PnsTools

#endif // REDUCEDPNSPROBLEMVIEW_H
