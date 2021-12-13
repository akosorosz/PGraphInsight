#include "algorithmabb.h"
#include "algorithmmsg.h"
#include "powerset.h"

namespace PnsTools {

void AlgorithmABB::insertSolutionSortedByObjectiveValue(const ABBSolution &solution)
{
	auto it=std::find_if(mSolutionStructures.begin(), mSolutionStructures.end(), [&solution](const ABBSolution &sol){
		return solution.objectiveValue<sol.objectiveValue;
	});
	mSolutionStructures.insert(it, solution);
}

void AlgorithmABB::abbRecursive(const PnsProblem &problem, const MaterialSet &pToBeProduced, const MaterialSet &pAlreadyProduced, const DecisionMapping &pDecisionMap, int parentStepId)
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
		decisionMap=problem.neutralExtension(pDecisionMap);
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

	double localBound=getBound(problem, includedUnits, excludedUnits);
	if (toBeProduced.empty())
	{
		if (localBound<mGlobalBound)
		{
			int solId=++mTotalSolCount;
			insertSolutionSortedByObjectiveValue({includedUnits,
										   problem.materialsConsumedByAnyOf(includedUnits) + problem.materialsProducedByAnyOf(includedUnits),
										   localBound});

			std::string additionalInfo;
			if (mSolutionStructures.size()>mMaxSolutionCount)
			{
				mSolutionStructures.pop_back();
				mGlobalBound=mSolutionStructures.back().objectiveValue;
				additionalInfo += std::string("\nGlobal bound updated to ") + std::to_string(mGlobalBound);
			}
			else if (mSolutionStructures.size()==mMaxSolutionCount) // It just reached the max count
			{
				mGlobalBound=mSolutionStructures.back().objectiveValue;
				additionalInfo += std::string("\nGlobal bound updated to ") + std::to_string(mGlobalBound);
			}

			if (localBound<mCurrentBest)
			{
				mSteps.push_back(StepOfAlgorithm(++mStepId, parentStepId, includedUnits, excludedUnits, localBound, subproblemCommentString + "\nSolution value: " + std::to_string(localBound) + "\nSolution structure #" + std::to_string(solId) + " found\nUnits: " + getUnitNamesString(includedUnits) + "\nThis is the new best solution" + additionalInfo));
				mCurrentBest=localBound;
			}
			else
			{
				mSteps.push_back(StepOfAlgorithm(++mStepId, parentStepId, includedUnits, excludedUnits, localBound, subproblemCommentString + "\nSolution value: " + std::to_string(localBound) + "\nSolution structure #" + std::to_string(solId) + " found\nUnits: " + getUnitNamesString(includedUnits) + additionalInfo));
			}
		}
		else
		{
			mSteps.push_back(StepOfAlgorithm(++mStepId, parentStepId, includedUnits, excludedUnits, localBound, subproblemCommentString + "\nSolution value: " + std::to_string(localBound) + "\nThis solution is fathomed, as it is worse than the bound."));
		}
		return;
	}

	if (localBound>mGlobalBound || (localBound>=mGlobalBound && mSolutionStructures.size()>=mMaxSolutionCount))
	{
		mSteps.push_back(StepOfAlgorithm(++mStepId, parentStepId, includedUnits, excludedUnits, localBound, subproblemCommentString + "\nLocal bound: " + std::to_string(localBound) + "\nThis subproblem is fathomed, as it is worse than the bound."));
		return;
	}

	Material selectedMaterial=toBeProduced.front();

	mSteps.push_back(StepOfAlgorithm(++mStepId, parentStepId, includedUnits, excludedUnits, localBound, subproblemCommentString + "\nLocal bound: " + std::to_string(localBound) + "\nSelected material for decision: " + selectedMaterial->name));
	int mystepId=mStepId;

	OperatingUnitSet canProduceSelectedMaterial = problem.unitsProducing(selectedMaterial);
	PowerSet<OperatingUnitSet> decisionOptions(canProduceSelectedMaterial);
	for (OperatingUnitSet decision : decisionOptions)
	{
		if (decision.empty()) continue;
		// consistency check
		OperatingUnitSet decisionInverse = canProduceSelectedMaterial - decision;
		bool isConsistent=std::all_of(decisionMap.begin(), decisionMap.end(), [&decision,&decisionInverse,&problem](const std::pair<const Material, OperatingUnitSet> &dec){
			return (decision & (problem.unitsProducing(dec.first)-dec.second)).empty() &&
					(decisionInverse & dec.second).empty();
		});
		if (isConsistent)
		{
			DecisionMapping newDecisionMap=decisionMap;
			newDecisionMap.insert({selectedMaterial, decision});
			abbRecursive(problem,
						 (toBeProduced + problem.materialsConsumedByAnyOf(decision))-(problem.rawMaterials() + alreadyProduced + selectedMaterial),
						 alreadyProduced + selectedMaterial,
						 newDecisionMap,
						 mystepId);
		}
	}
}

double AlgorithmABB::getBound(const PnsProblem &, const OperatingUnitSet &includedUnits, const OperatingUnitSet &)
{
	// For now this is fix bound on weight, and the bound is the sum of the included units
	if (mEvaluation==EVAL_NONE) return 0.0;
	else if (mEvaluation==EVAL_SUMWEIGHT) return getSumOfIncludedWeights(includedUnits);
	else return 0.0;
}

double AlgorithmABB::getSumOfIncludedWeights(const OperatingUnitSet &includedUnits)
{
	double weightSum=0.0;
	for (const OperatingUnit &unit : includedUnits)
		weightSum += unit->weight;
	return weightSum;
}

AlgorithmABB::AlgorithmABB(const PnsProblem &problem, unsigned int maxSolutions, EvaluationType evaluation, unsigned int accelerations):
	AlgorithmBase(problem),
	mMaxSolutionCount(maxSolutions),
	mEvaluation(evaluation),
	mUseNeutralExtension(accelerations&ACCEL_NEUTRAL_EXTENSION)
{
	if (mMaxSolutionCount<1) mMaxSolutionCount=1;
}

void AlgorithmABB::run()
{
	mSolutionStructures.clear();
	mSteps.clear();
	mStepId=-1;
	int parentStepId=-1;
	mTotalSolCount=0;
	mCurrentBest=std::numeric_limits<double>::infinity();
	mGlobalBound=std::numeric_limits<double>::infinity();

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

	abbRecursive(problem, problem.products(), MaterialSet(), {}, parentStepId);
}

const std::list<AlgorithmABB::ABBSolution> &AlgorithmABB::getSolutionStructures() const
{
	return mSolutionStructures;
}

const std::list<StepOfAlgorithm> &AlgorithmABB::getSteps() const
{
	return mSteps;
}

} // namespace PnsTools

