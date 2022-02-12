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
		// Find a material for which there is exactly 1 possible decision, i.e. either:
		// - All of the units producing it are already included or excluded
		//   - Note that in the special case, where all units are excluded, there is no consistent decision, so no decision is made
		// - All of the units producing it are already excluded, except 1, which is still free
		// - The number of units producing it and already included has reached the maximum parallel production limit
		//   - Note that only the equality is accepted, if it surpassed the number, then there is no appropriate decision
		auto materialIt=std::find_if(toBeProduced.begin(), toBeProduced.end(), [&includedUnits,&excludedUnits,&maxParallelProduction,this](Material m){
			OperatingUnitSet producingM=this->unitsProducing(m);
			OperatingUnitSet producingNotDecided=producingM-excludedUnits-includedUnits;
			OperatingUnitSet alreadyAdded=producingM&includedUnits;
			return (alreadyAdded.size()>0 && producingNotDecided.size()==0) || (alreadyAdded.size()==0 && producingNotDecided.size()==1) || alreadyAdded.size()==maxParallelProduction;
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
