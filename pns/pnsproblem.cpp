#include "pnsproblem.h"

namespace PnsTools {

std::string getMaterialNamesString(const MaterialSet &materials)
{
	std::string str;
	bool first=true;
	for (Material material : materials)
	{
		if (!first) str+=", ";
		else first=false;
		str+=material->name;
	}
	return str;
}

std::string getUnitNamesString(const OperatingUnitSet &units)
{
	std::string str;
	bool first=true;
	for (OperatingUnit unit : units)
	{
		if (!first) str+=", ";
		else first=false;
		str+=unit->name;
	}
	return str;
}

MaterialSet PnsProblem::toBeProducedInDecisionMapping(const DecisionMapping &decisionMap) const
{
	MaterialSet toBeProduced=this->products();
	for (const auto &decision : decisionMap)
		toBeProduced+=this->materialsConsumedByAnyOf(decision.second);
	toBeProduced-=this->rawMaterials();
	toBeProduced-=alreadyProducedInDecisionMapping(decisionMap);
	return toBeProduced;
}

MaterialSet PnsProblem::alreadyProducedInDecisionMapping(const DecisionMapping &decisionMap) const
{
	MaterialSet alreadyProduced;
	for (const auto &decision : decisionMap)
		alreadyProduced+=decision.first;
	return alreadyProduced;
}

OperatingUnitSet PnsProblem::includedUnitsInDecisionMapping(const DecisionMapping &decisionMap) const
{
	OperatingUnitSet includedUnits;
	for (const auto &decision : decisionMap)
		includedUnits+=decision.second;
	return includedUnits;
}

OperatingUnitSet PnsProblem::excludedUnitsInDecisionMapping(const DecisionMapping &decisionMap) const
{
	OperatingUnitSet excludedUnits;
	for (const auto &decision : decisionMap)
		excludedUnits+=this->unitsProducing(decision.first)-decision.second;
	return excludedUnits;
}

DecisionMapping PnsProblem::neutralExtension(const DecisionMapping &decisionMap, unsigned int maxParallelProduction) const
{
	DecisionMapping extended=decisionMap;
	bool newRound=true;
	MaterialSet alreadyProduced=alreadyProducedInDecisionMapping(extended);
	MaterialSet toBeProduced=toBeProducedInDecisionMapping(extended);
	OperatingUnitSet includedUnits=includedUnitsInDecisionMapping(extended);
	OperatingUnitSet excludedUnits=excludedUnitsInDecisionMapping(extended);
	while (newRound)
	{
		auto materialIt=std::find_if(toBeProduced.begin(), toBeProduced.end(), [&includedUnits,&excludedUnits,&maxParallelProduction,this](Material m){
			OperatingUnitSet producingM=this->unitsProducing(m);
			OperatingUnitSet canBeAdded=producingM-excludedUnits;
			OperatingUnitSet alreadyAdded=producingM&includedUnits;
			return canBeAdded.size()==0 || (alreadyAdded.size()==0 && canBeAdded.size()==1) || alreadyAdded.size()==maxParallelProduction;
		});
		if (materialIt==toBeProduced.end()) newRound=false;
		else
		{
			newRound=true;
			if ((this->unitsProducing(*materialIt)&includedUnits).size()==maxParallelProduction)
				extended.insert({*materialIt,this->unitsProducing(*materialIt)&includedUnits});
			else
				extended.insert({*materialIt,this->unitsProducing(*materialIt)-excludedUnits});
			alreadyProduced+=*materialIt;
			toBeProduced=toBeProducedInDecisionMapping(extended);
			includedUnits+=this->unitsProducing(*materialIt)-excludedUnits;
		}
	}
	return extended;
}

} // namespace PnsTools
