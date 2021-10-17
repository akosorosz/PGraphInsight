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

void AlgorithmABB::abbRecursive(const ReducedPnsProblemView &problem, const MaterialSet &toBeProduced, const MaterialSet &alreadyProduced, const DecisionMapping &decisionMap, const OperatingUnitSet &includedUnits, const OperatingUnitSet &excludedUnits, int parentStepId)
{
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
				mSteps.push_back(StepOfAlgorithm(++mStepId, parentStepId, includedUnits, excludedUnits, localBound, "Materials to be produced: " + getMaterialNamesString(toBeProduced) + "\nMaterials already decided: " + getMaterialNamesString(alreadyProduced) + "\nIncluded units: " + getUnitNamesString(includedUnits) + "\nExcluded units: " + getUnitNamesString(excludedUnits) + "\nSolution value: " + std::to_string(localBound) + "\nSolution structure #" + std::to_string(solId) + " found\nUnits: " + getUnitNamesString(includedUnits) + "\nThis is the new best solution" + additionalInfo));
				mCurrentBest=localBound;
			}
			else
			{
				mSteps.push_back(StepOfAlgorithm(++mStepId, parentStepId, includedUnits, excludedUnits, localBound, "Materials to be produced: " + getMaterialNamesString(toBeProduced) + "\nMaterials already decided: " + getMaterialNamesString(alreadyProduced) + "\nIncluded units: " + getUnitNamesString(includedUnits) + "\nExcluded units: " + getUnitNamesString(excludedUnits) + "\nSolution value: " + std::to_string(localBound) + "\nSolution structure #" + std::to_string(solId) + " found\nUnits: " + getUnitNamesString(includedUnits) + additionalInfo));
			}
		}
		else
		{
			mSteps.push_back(StepOfAlgorithm(++mStepId, parentStepId, includedUnits, excludedUnits, localBound, "Materials to be produced: " + getMaterialNamesString(toBeProduced) + "\nMaterials already decided: " + getMaterialNamesString(alreadyProduced) + "\nIncluded units: " + getUnitNamesString(includedUnits) + "\nExcluded units: " + getUnitNamesString(excludedUnits) + "\nSolution value: " + std::to_string(localBound) + "\nThis solution is fathomed, as it is worse than the bound."));
		}
		return;
	}

	if (localBound>mGlobalBound || (localBound>=mGlobalBound && mSolutionStructures.size()>=mMaxSolutionCount))
	{
		mSteps.push_back(StepOfAlgorithm(++mStepId, parentStepId, includedUnits, excludedUnits, localBound, "Materials to be produced: " + getMaterialNamesString(toBeProduced) + "\nMaterials already decided: " + getMaterialNamesString(alreadyProduced) + "\nIncluded units: " + getUnitNamesString(includedUnits) + "\nExcluded units: " + getUnitNamesString(excludedUnits) + "\nLocal bound: " + std::to_string(localBound) + "\nThis subproblem is fathomed, as it is worse than the bound."));
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


	mSteps.push_back(StepOfAlgorithm(++mStepId, parentStepId, includedUnits, excludedUnits, localBound, "Materials to be produced: " + getMaterialNamesString(toBeProduced) + "\nMaterials already decided: " + getMaterialNamesString(alreadyProduced) + "\nIncluded units: " + getUnitNamesString(includedUnits) + "\nExcluded units: " + getUnitNamesString(excludedUnits) + "\nLocal bound: " + std::to_string(localBound) + decisionMapString + "\nSelected material for decision: " + selectedMaterial->name));
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
			abbRecursive(problem,
						 (toBeProduced + problem.materialsConsumedByAnyOf(decision))-(problem.rawMaterials() + alreadyProduced + selectedMaterial),
						 alreadyProduced + selectedMaterial,
						 newDecisionMap,
						 includedUnits + decision,
						 excludedUnits + decisionInverse,
						 mystepId);
		}
	}
}

double AlgorithmABB::getBound(const ReducedPnsProblemView &, const OperatingUnitSet &includedUnits, const OperatingUnitSet &)
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

AlgorithmABB::AlgorithmABB(const PnsProblem &problem, unsigned int maxSolutions, EvaluationType evaluation):
	AlgorithmBase(problem),
	mOnlyConsiderTheseUnits(problem.operatingUnits()),
	mMaxSolutionCount(maxSolutions),
	mEvaluation(evaluation)
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

	abbRecursive(problem, problem.products(), MaterialSet(), {}, OperatingUnitSet(), OperatingUnitSet(), parentStepId);
}

void AlgorithmABB::onlyConsiderTheseUnits(const OperatingUnitSet &units)
{
	mOnlyConsiderTheseUnits=units;
}

void AlgorithmABB::excludeTheseUnits(const OperatingUnitSet &units)
{
	mExcludeTheseUnits=units;
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

