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

class MaterialIdExistsException : public std::exception{
	std::string errorStr;
public:
	MaterialIdExistsException(int id):
		errorStr("Material id "+std::to_string(id)+" already exists in the PNS problem")
	{}
	const char *what() const noexcept override{
		return errorStr.c_str();
	}
};

class OperatingUnitIdExistsException : public std::exception{
	std::string errorStr;
public:
	OperatingUnitIdExistsException(int id):
		errorStr("Operating unit id "+std::to_string(id)+" already exists in the PNS problem")
	{}
	const char *what() const noexcept override{
		return errorStr.c_str();
	}
};

class MaterialIdDoesNotExistException : public std::exception{
	std::string errorStr;
public:
	MaterialIdDoesNotExistException(int id):
		errorStr("Material id "+std::to_string(id)+" does not exist in the PNS problem")
	{}
	const char *what() const noexcept override{
		return errorStr.c_str();
	}
};

class OperatingUnitIdDoesNotExistException : public std::exception{
	std::string errorStr;
public:
	OperatingUnitIdDoesNotExistException(int id):
		errorStr("Operating unit id "+std::to_string(id)+" does not exist in the PNS problem")
	{}
	const char *what() const noexcept override{
		return errorStr.c_str();
	}
};


class PnsProblem
{
	bool mIsEmpty=true;
	std::list<MaterialData> mMaterialDataList;
	std::list<OperatingUnitData> mOperatingUnitDataList;

	MaterialSet mMaterials, mRawMaterials, mProducts, mIntermediates;
	OperatingUnitSet mOperatingUnits;

	std::map<Material,OperatingUnitSet> mUnitsThatProduceMaterial;
	std::map<Material,OperatingUnitSet> mUnitsThatConsumeMaterial;
	std::map<OperatingUnit,MaterialSet> mMaterialsProducedByUnit;
	std::map<OperatingUnit,MaterialSet> mMaterialsConsumedByUnit;

	std::map<int,Material> mMaterialIdToPointer;
	std::map<int,OperatingUnit> mOperatingUnitIdToPointer;
	std::map<OperatingUnit,std::map<Material,double>> mInputFlowRates;
	std::map<OperatingUnit,std::map<Material,double>> mOutputFlowRates;
public:
	void clear();
	void addMaterialData(const MaterialData &newMaterialData);
	void addOperatingUnitData(const OperatingUnitData &newOperatingUnitData);
	void addUnitMaterialConnection(int unitId, int materialId, bool isInput, double flowRate);

	bool isEmpty() const;
	const MaterialSet &materials() const;
	const MaterialSet &rawMaterials() const;
	const MaterialSet &products() const;
	const MaterialSet &intermediates() const;
	const OperatingUnitSet &operatingUnits() const;
	OperatingUnitSet unitsProducing(const Material &material) const;
	OperatingUnitSet unitsConsuming(const Material &material) const;
	MaterialSet materialsProducedBy(const OperatingUnit &unit) const;
	MaterialSet materialsConsumedBy(const OperatingUnit &unit) const;
	OperatingUnitSet unitsProducingAnyOf(const MaterialSet &materials) const;
	OperatingUnitSet unitsConsumingAnyOf(const MaterialSet &materials) const;
	MaterialSet materialsProducedByAnyOf(const OperatingUnitSet &units) const;
	MaterialSet materialsConsumedByAnyOf(const OperatingUnitSet &units) const;

	std::string dumpData() const;
};

} // namespace PnsTools

#endif // PNSPROBLEM_H
