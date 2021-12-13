#ifndef PNSPROBLEM_H
#define PNSPROBLEM_H

#include <string>
#include <list>
#include <map>
#include <exception>
#include "extendedset.h"

namespace PnsTools {

enum MaterialType{
	INTERMEDIATE=0,
	RAW,
	PRODUCT
};

struct MaterialData
{
	int id;
	std::string name;
	MaterialType type;
	double unitPrice;
	double minFlow;
	double maxFlow;

	MaterialData():
		id(-1),name(""),type(INTERMEDIATE),unitPrice(0.0),minFlow(0.0),maxFlow(10000000.0)
	{}

	MaterialData(int id, const std::string &name, MaterialType type,
				 double unitPrice=0.0, double minFlow=0.0, double maxFlow=10000000.0):
		id(id),name(name),type(type),unitPrice(unitPrice),minFlow(minFlow),maxFlow(maxFlow)
	{}
};

struct OperatingUnitData
{
	int id;
	std::string name;
	double weight;
	double fixCapitalCost;
	double propCapitalCost;
	double fixOperatingCost;
	double propOperatingCost;
	double minSize;
	double maxSize;

	OperatingUnitData():
		id(-1),name(""),weight(0.0),fixCapitalCost(0.0),propCapitalCost(0.0),
		fixOperatingCost(0.0),propOperatingCost(0.0),minSize(0.0),maxSize(10000000.0)
	{}

	OperatingUnitData(int id, const std::string &name, double weight=0.0,
					  double fixCapitalCost=0.0, double propCapitalCost=0.0,
					  double fixOperatingCost=0.0, double propOperatingCost=0.0,
					  double minSize=0.0, double maxSize=10000000.0):
		id(id),name(name),weight(weight),fixCapitalCost(fixCapitalCost),propCapitalCost(propCapitalCost),
		fixOperatingCost(fixOperatingCost),propOperatingCost(propOperatingCost),minSize(minSize),maxSize(maxSize)
	{}
};

using Material=MaterialData*;
using OperatingUnit=OperatingUnitData*;

struct materialCompare{
	bool operator()(const Material &m1, const Material &m2) const{
		return m1->id<m2->id;
	}
};
struct operatingUnitCompare{
	bool operator()(const OperatingUnit &o1, const OperatingUnit &o2) const{
		return o1->id<o2->id;
	}
};

using MaterialSet=ExtendedSet<Material,materialCompare>;
using OperatingUnitSet=ExtendedSet<OperatingUnit,operatingUnitCompare>;

using DecisionMapping = std::map<Material, OperatingUnitSet, materialCompare>;

std::string getMaterialNamesString(const MaterialSet &materials);
std::string getUnitNamesString(const OperatingUnitSet &units);

class PnsProblem
{
public:
	virtual ~PnsProblem() = default;
	virtual bool isEmpty() const =0;
	virtual const MaterialSet &materials() const =0;
	virtual const MaterialSet &rawMaterials() const =0;
	virtual const MaterialSet &products() const =0;
	virtual const MaterialSet &intermediates() const =0;
	virtual const OperatingUnitSet &operatingUnits() const =0;
	virtual OperatingUnitSet unitsProducing(const Material &material) const =0;
	virtual OperatingUnitSet unitsConsuming(const Material &material) const =0;
	virtual MaterialSet materialsProducedBy(const OperatingUnit &unit) const =0;
	virtual MaterialSet materialsConsumedBy(const OperatingUnit &unit) const =0;
	virtual OperatingUnitSet unitsProducingAnyOf(const MaterialSet &materials) const =0;
	virtual OperatingUnitSet unitsConsumingAnyOf(const MaterialSet &materials) const =0;
	virtual MaterialSet materialsProducedByAnyOf(const OperatingUnitSet &units) const =0;
	virtual MaterialSet materialsConsumedByAnyOf(const OperatingUnitSet &units) const =0;

	MaterialSet toBeProducedInDecisionMapping(const DecisionMapping &decisionMap) const;
	MaterialSet alreadyProducedInDecisionMapping(const DecisionMapping &decisionMap) const;
	OperatingUnitSet includedUnitsInDecisionMapping(const DecisionMapping &decisionMap) const;
	OperatingUnitSet excludedUnitsInDecisionMapping(const DecisionMapping &decisionMap) const;
	DecisionMapping neutralExtension(const DecisionMapping &decisionMap, unsigned int maxParallelProduction=10000000) const;
};

} // namespace PnsTools

#endif // PNSPROBLEM_H
