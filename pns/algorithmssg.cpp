#include "algorithmssg.h"
#include "algorithmmsg.h"
#include "powerset.h"

namespace PnsTools {

void AlgorithmSSG::ssgRecursive(const ReducedPnsProblemView &problem, const MaterialSet &toBeProduced, const MaterialSet &alreadyProduced, const DecisionMapping &decisionMap, const OperatingUnitSet &includedUnits, const OperatingUnitSet &excludedUnits, int parentStepId)
{
	if (toBeProduced.empty())
	{
		int solId=mSolutionStructures.size()+1;
		mSolutionStructures.push_back({includedUnits,
									   problem.materialsConsumedByAnyOf(includedUnits) + problem.materialsProducedByAnyOf(includedUnits)});
		mSteps.push_back(StepOfAlgorithm(++mStepId, parentStepId, includedUnits, excludedUnits, 0.0, "Materials to be produced: " + getMaterialNamesString(toBeProduced) + "\nMaterials already decided: " + getMaterialNamesString(alreadyProduced) + "\nIncluded units: " + getUnitNamesString(includedUnits) + "\nExcluded units: " + getUnitNamesString(excludedUnits) + "\nSolution structure #" + std::to_string(solId) + " found\nUnits: " + getUnitNamesString(includedUnits)));
		return;
	}

	Material selectedMaterial=toBeProduced.front();

	std::string decisionMapString("\nCurrent decision mapping:");
	if (!decisionMap.empty())
	{
		for (const auto &dec : decisionMap)
			decisionMapString += "\n  "+dec.first->name+": "+getUnitNamesString(dec.second);
	}
	else
	{
		decisionMapString += " empty";
	}

	mSteps.push_back(StepOfAlgorithm(++mStepId, parentStepId, includedUnits, excludedUnits, 0.0, "Materials to be produced: " + getMaterialNamesString(toBeProduced) + "\nMaterials already decided: " + getMaterialNamesString(alreadyProduced) + "\nIncluded units: " + getUnitNamesString(includedUnits) + "\nExcluded units: " + getUnitNamesString(excludedUnits) + decisionMapString + "\nSelected material for decision: " + selectedMaterial->name));
	int mystepId=mStepId;

	OperatingUnitSet canProduceSelectedMaterial = problem.unitsProducing(selectedMaterial);
	PowerSet<OperatingUnitSet> decisionOptions(canProduceSelectedMaterial);
	for (OperatingUnitSet decision : decisionOptions)
	{
		if (decision.empty()) continue;
		// consistency check
		OperatingUnitSet decisionInverse = canProduceSelectedMaterial - decision;
		bool isConsistent=std::all_of(decisionMap.begin(), decisionMap.end(), [&decision,&decisionInverse,&problem](const std::pair<Material, OperatingUnitSet> &dec){
			return (decision & (problem.unitsProducing(dec.first)-dec.second)).empty() &&
					(decisionInverse & dec.second).empty();
		});
		if (isConsistent)
		{
			DecisionMapping newDecisionMap=decisionMap;
			newDecisionMap.insert({selectedMaterial, decision});
			ssgRecursive(problem,
						 (toBeProduced + problem.materialsConsumedByAnyOf(decision))-(problem.rawMaterials() + alreadyProduced + selectedMaterial),
						 alreadyProduced + selectedMaterial,
						 newDecisionMap,
						 includedUnits + decision,
						 excludedUnits + decisionInverse,
						 mystepId);
		}
	}
}

AlgorithmSSG::AlgorithmSSG(const PnsProblem &problem):
	AlgorithmBase(problem)
{
}

void AlgorithmSSG::run()
{
	mSolutionStructures.clear();
	mSteps.clear();
	mStepId=-1;
	int parentStepId=-1;

	// First step: algorithm MSG
	AlgorithmMSG msg(mProblem);
	msg.onlyConsiderTheseUnits(mOnlyConsiderTheseUnits);
	msg.excludeTheseUnits(mExcludeTheseUnits);
	msg.saveSteps(false);
	msg.run();
	OperatingUnitSet msgUnits=msg.getUnitsOfMaximalStructure();

	ReducedPnsProblemView problem(mProblem);
	problem.setBaseUnitSet(msgUnits);
	if (mSaveSteps) mSteps.push_back(StepOfAlgorithm(++mStepId, parentStepId, msgUnits, OperatingUnitSet(), 0.0, std::string("Before starting SSG, Step #0 is to generate the maximal structure.\nUnits in maximal structure: " + getUnitNamesString(msgUnits))));
	parentStepId=mStepId;

	if (msgUnits.empty())
	{
		if (mSaveSteps) mSteps.push_back(StepOfAlgorithm(++mStepId, parentStepId, OperatingUnitSet(), OperatingUnitSet(), 0.0, std::string("Finished: No maximal structure.")));
		return;
	}

	ssgRecursive(problem, problem.products(), MaterialSet(), {}, OperatingUnitSet(), OperatingUnitSet(), parentStepId);
}

const std::list<std::pair<OperatingUnitSet, MaterialSet> > &AlgorithmSSG::getSolutionStructures() const
{
	return mSolutionStructures;
}

const std::list<StepOfAlgorithm> &AlgorithmSSG::getSteps() const
{
	return mSteps;
}

} // namespace PnsTools
