#ifndef REDUCEDPNSPROBLEMVIEW_H
#define REDUCEDPNSPROBLEMVIEW_H

#include "pnsproblem.h"

namespace PnsTools {

class ReducedPnsProblemView : public PnsProblem
{
	const PnsProblem &mProblem;
	OperatingUnitSet mBaseUnitSet;
public:
	ReducedPnsProblemView(const PnsProblem &problem);
	bool isEmpty() const override;
	const MaterialSet &materials() const override;
	const MaterialSet &rawMaterials() const override;
	const MaterialSet &products() const override;
	const MaterialSet &intermediates() const override;
	const OperatingUnitSet &operatingUnits() const override;
	OperatingUnitSet unitsProducing(const Material &material) const override;
	OperatingUnitSet unitsConsuming(const Material &material) const override;
	MaterialSet materialsProducedBy(const OperatingUnit &unit) const override;
	MaterialSet materialsConsumedBy(const OperatingUnit &unit) const override;
	OperatingUnitSet unitsProducingAnyOf(const MaterialSet &materials) const override;
	OperatingUnitSet unitsConsumingAnyOf(const MaterialSet &materials) const override;
	MaterialSet materialsProducedByAnyOf(const OperatingUnitSet &units) const override;
	MaterialSet materialsConsumedByAnyOf(const OperatingUnitSet &units) const override;

	void setBaseUnitSet(const OperatingUnitSet &units);
};

} // namespace PnsTools

#endif // REDUCEDPNSPROBLEMVIEW_H
