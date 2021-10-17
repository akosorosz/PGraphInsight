#include "reducedpnsproblemview.h"

namespace PnsTools {

ReducedPnsProblemView::ReducedPnsProblemView(const PnsProblem &problem):
	mProblem(problem),
	mBaseUnitSet(problem.operatingUnits())
{
}

bool ReducedPnsProblemView::isEmpty() const
{
	return mProblem.isEmpty();
}

const MaterialSet &ReducedPnsProblemView::materials() const
{
	return mProblem.materials();
}

const MaterialSet &ReducedPnsProblemView::rawMaterials() const
{
	return mProblem.rawMaterials();
}

const MaterialSet &ReducedPnsProblemView::products() const
{
	return mProblem.products();
}

const MaterialSet &ReducedPnsProblemView::intermediates() const
{
	return mProblem.intermediates();
}

const OperatingUnitSet &ReducedPnsProblemView::operatingUnits() const
{
	return mBaseUnitSet;
}

OperatingUnitSet ReducedPnsProblemView::unitsProducing(const Material &material) const
{
	return mProblem.unitsProducing(material) & mBaseUnitSet;
}

OperatingUnitSet ReducedPnsProblemView::unitsConsuming(const Material &material) const
{
	return mProblem.unitsConsuming(material) & mBaseUnitSet;
}

MaterialSet ReducedPnsProblemView::materialsProducedBy(const OperatingUnit &unit) const
{
	if (mBaseUnitSet.contains(unit)) return mProblem.materialsProducedBy(unit);
	else return MaterialSet();
}

MaterialSet ReducedPnsProblemView::materialsConsumedBy(const OperatingUnit &unit) const
{
	if (mBaseUnitSet.contains(unit)) return mProblem.materialsConsumedBy(unit);
	else return MaterialSet();
}

OperatingUnitSet ReducedPnsProblemView::unitsProducingAnyOf(const MaterialSet &materials) const
{
	return mProblem.unitsProducingAnyOf(materials) & mBaseUnitSet;
}

OperatingUnitSet ReducedPnsProblemView::unitsConsumingAnyOf(const MaterialSet &materials) const
{
	return mProblem.unitsConsumingAnyOf(materials) & mBaseUnitSet;
}

MaterialSet ReducedPnsProblemView::materialsProducedByAnyOf(const OperatingUnitSet &units) const
{
	return mProblem.materialsProducedByAnyOf(units&mBaseUnitSet);
}

MaterialSet ReducedPnsProblemView::materialsConsumedByAnyOf(const OperatingUnitSet &units) const
{
	return mProblem.materialsConsumedByAnyOf(units&mBaseUnitSet);
}

void ReducedPnsProblemView::setBaseUnitSet(const OperatingUnitSet &units)
{
	mBaseUnitSet=units;
}

} // namespace PnsTools
