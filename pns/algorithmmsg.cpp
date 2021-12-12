#include "algorithmmsg.h"
#include "reducedpnsproblemview.h"

namespace PnsTools {

AlgorithmMSG::AlgorithmMSG(const PnsProblem &problem):
	AlgorithmBase(problem)
{
}

void AlgorithmMSG::run()
{
	mUnitsInMaximalStructure.clear();
	mMaterialsInMaximalStructure.clear();
	mSteps.clear();
	ReducedPnsProblemView problem(mProblem);
	problem.setBaseUnitSet((mProblem.operatingUnits()&mOnlyConsiderTheseUnits) - mExcludeTheseUnits);
	int stepId=0;
	int parentStepId=-1;

	// Reduction phase: initialization
	OperatingUnitSet unitsToBeRemoved = problem.unitsProducingAnyOf(problem.rawMaterials());
	OperatingUnitSet units = problem.operatingUnits() - unitsToBeRemoved;
	MaterialSet materials = problem.materialsConsumedByAnyOf(units) + problem.materialsProducedByAnyOf(units);
	MaterialSet nonProducedMaterials = materials - problem.rawMaterials() - problem.materialsProducedByAnyOf(units);
	if (mSaveSteps) mSteps.push_back(StepOfAlgorithm(++stepId, parentStepId, OperatingUnitSet(), unitsToBeRemoved, 0.0, std::string("Reduction phase\nRemoved units (producing raw materials): ")+getUnitNamesString(unitsToBeRemoved)+std::string("\nNot produced materials: ")+getMaterialNamesString(nonProducedMaterials)));
	parentStepId=stepId;
	if (! problem.products().isSubsetOf(materials))
	{
		if (mSaveSteps) mSteps.push_back(StepOfAlgorithm(++stepId, parentStepId, OperatingUnitSet(), unitsToBeRemoved, 0.0, std::string("Finished\nNo solution: some products cannot be produced")));
		parentStepId=stepId;
		return; // No solution: some products cannot be produced
	}

	// Reduction phase: iteration
	while (! nonProducedMaterials.empty())
	{
		Material selectedMaterial = nonProducedMaterials.front();
		unitsToBeRemoved = problem.unitsConsuming(selectedMaterial);
		units -= unitsToBeRemoved;
		MaterialSet newSources = (problem.materialsConsumedByAnyOf(units) - problem.materialsProducedByAnyOf(units)) &
				problem.materialsProducedByAnyOf(unitsToBeRemoved);
		materials = problem.materialsConsumedByAnyOf(units) + problem.materialsProducedByAnyOf(units);
		nonProducedMaterials = (nonProducedMaterials & materials) + newSources;
		if (mSaveSteps) mSteps.push_back(StepOfAlgorithm(++stepId, parentStepId, OperatingUnitSet(), problem.operatingUnits()-units, 0.0, std::string("Reduction phase\nSelected material: " + selectedMaterial->name + "\nNewly removed units: ")+getUnitNamesString(unitsToBeRemoved)+std::string("\nNot produced materials: ")+getMaterialNamesString(nonProducedMaterials)));
		parentStepId=stepId;
		if (! problem.products().isSubsetOf(materials))
		{
			if (mSaveSteps) mSteps.push_back(StepOfAlgorithm(++stepId, parentStepId, OperatingUnitSet(), problem.operatingUnits()-units, 0.0, std::string("Finished\nNo solution: some products cannot be produced")));
			parentStepId=stepId;
			return; // No solution: some products cannot be produced
		}
	}
	OperatingUnitSet allExcludedUnits=problem.operatingUnits()-units;

	ReducedPnsProblemView problemComp(mProblem);
	problemComp.setBaseUnitSet(problem.operatingUnits()-allExcludedUnits);

	// Composition phase: initialization
	MaterialSet materialsToBeProduced = problemComp.products();
	MaterialSet includedMaterials;
	OperatingUnitSet includedUnits;
	if (mSaveSteps) mSteps.push_back(StepOfAlgorithm(++stepId, parentStepId, includedUnits, allExcludedUnits, 0.0, std::string("Construction phase\nMaterials to be produced: ")+getMaterialNamesString(materialsToBeProduced)));
	parentStepId=stepId;

	// Composition phase: iteration
	while (! materialsToBeProduced.empty())
	{
		Material selectedMaterial = materialsToBeProduced.front();
		includedMaterials += selectedMaterial;
		OperatingUnitSet newUnits = problemComp.unitsProducing(selectedMaterial);
		includedUnits += newUnits;
		materialsToBeProduced = (materialsToBeProduced + problemComp.materialsConsumedByAnyOf(newUnits)) -
				(problemComp.rawMaterials() + includedMaterials);
		if (mSaveSteps) mSteps.push_back(StepOfAlgorithm(++stepId, parentStepId, includedUnits, allExcludedUnits, 0.0, std::string("Construction phase\nSelected material: " + selectedMaterial->name + "\nNewly included units: ")+getUnitNamesString(newUnits)+std::string("\nMaterials to be produced: ")+getMaterialNamesString(materialsToBeProduced)));
		parentStepId=stepId;
	}

	// End, save the solution
	if (mSaveSteps) mSteps.push_back(StepOfAlgorithm(++stepId, parentStepId, includedUnits, allExcludedUnits, 0.0, std::string("Finished\nMaximal structure: ")+getUnitNamesString(includedUnits)));
	parentStepId=stepId;
	mUnitsInMaximalStructure=includedUnits;
	mMaterialsInMaximalStructure=problemComp.materialsConsumedByAnyOf(includedUnits) + problemComp.materialsProducedByAnyOf(includedUnits);
}

const OperatingUnitSet &AlgorithmMSG::getUnitsOfMaximalStructure() const
{
	return mUnitsInMaximalStructure;
}

MaterialSet AlgorithmMSG::getMaterialsOfMaximalStructure() const
{
	return mMaterialsInMaximalStructure;
}

const std::list<StepOfAlgorithm> &AlgorithmMSG::getSteps() const
{
	return mSteps;
}

} // namespace PnsTools
