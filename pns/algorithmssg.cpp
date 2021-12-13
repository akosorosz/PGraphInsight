#include "algorithmssg.h"
#include "algorithmmsg.h"
#include "powerset.h"

namespace PnsTools {

void AlgorithmSSG::ssgRecursive(const ReducedPnsProblemView &problem, const MaterialSet &pToBeProduced, const MaterialSet &pAlreadyProduced, const DecisionMapping &pDecisionMap, int parentStepId)
{
	MaterialSet toBeProduced=pToBeProduced;
	MaterialSet alreadyProduced=pAlreadyProduced;
	DecisionMapping decisionMap=pDecisionMap;
	OperatingUnitSet includedUnits=problem.includedUnitsInDecisionMapping(decisionMap);
	OperatingUnitSet excludedUnits=problem.excludedUnitsInDecisionMapping(decisionMap);
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
	std::string subproblemCommentString("Materials to be produced: " + getMaterialNamesString(toBeProduced) + "\nMaterials already decided: " + getMaterialNamesString(alreadyProduced) + "\nIncluded units: " + getUnitNamesString(includedUnits) + "\nExcluded units: " + getUnitNamesString(excludedUnits) + decisionMapString);

	if (mUseNeutralExtension)
	{
		decisionMap=problem.neutralExtension(pDecisionMap, mMaxParallelProduction);
		if (decisionMap.size()==pDecisionMap.size())
		{
			subproblemCommentString.append("\nNeutral extension: No effect");
		}
		else
		{
			toBeProduced=problem.toBeProducedInDecisionMapping(decisionMap);
			alreadyProduced=problem.alreadyProducedInDecisionMapping(decisionMap);
			includedUnits=problem.includedUnitsInDecisionMapping(decisionMap);
			excludedUnits=problem.excludedUnitsInDecisionMapping(decisionMap);
			std::string newDecisionMapString("\n    Decision mapping:");
			for (const auto &dec : decisionMap)
				newDecisionMapString += "\n      "+dec.first->name+": "+getUnitNamesString(dec.second);

			subproblemCommentString.append("\nNeutral extension: extended with " + std::to_string(decisionMap.size()-pDecisionMap.size()) + " decisions. New data:" + "\n    Materials to be produced: " + getMaterialNamesString(toBeProduced) + "\n    Materials already decided: " + getMaterialNamesString(alreadyProduced) + "\n    Included units: " + getUnitNamesString(includedUnits) + "\n    Excluded units: " + getUnitNamesString(excludedUnits) + newDecisionMapString);
		}
	}

	if (toBeProduced.empty())
	{
		int solId=mSolutionStructures.size()+1;
		mSolutionStructures.push_back({includedUnits,
									   problem.materialsConsumedByAnyOf(includedUnits) + problem.materialsProducedByAnyOf(includedUnits)});
		mSteps.push_back(StepOfAlgorithm(++mStepId, parentStepId, includedUnits, excludedUnits, 0.0, subproblemCommentString + "\nSolution structure #" + std::to_string(solId) + " found\nUnits: " + getUnitNamesString(includedUnits)));
		return;
	}

	Material selectedMaterial=toBeProduced.front();

	mSteps.push_back(StepOfAlgorithm(++mStepId, parentStepId, includedUnits, excludedUnits, 0.0, subproblemCommentString + "\nSelected material for decision: " + selectedMaterial->name));
	int mystepId=mStepId;

	OperatingUnitSet canProduceSelectedMaterial = problem.unitsProducing(selectedMaterial);
	PowerSet<OperatingUnitSet> decisionOptions(canProduceSelectedMaterial);
	for (OperatingUnitSet decision : decisionOptions)
	{
		if (decision.empty()) continue;
		if (decision.size()>mMaxParallelProduction) continue;
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
						 mystepId);
		}
	}
}

AlgorithmSSG::AlgorithmSSG(const PnsProblem &problem, unsigned int accelerations, unsigned int maxParallelProduction):
	AlgorithmBase(problem),
	mUseNeutralExtension(accelerations&ACCEL_NEUTRAL_EXTENSION),
	mMaxParallelProduction(maxParallelProduction)
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

	ssgRecursive(problem, problem.products(), MaterialSet(), {}, parentStepId);
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
