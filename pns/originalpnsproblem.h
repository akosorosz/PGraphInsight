#ifndef ORIGINALPNSPROBLEM_H
#define ORIGINALPNSPROBLEM_H

#include "pnsproblem.h"

namespace PnsTools {

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

class OriginalPnsProblem : public PnsProblem
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

	bool isEmpty() const override;
	const MaterialSet &materials() const override;
	const MaterialSet &rawMaterials() const override;
	const MaterialSet &products() const override;
	const MaterialSet &intermediates() const override;
	const OperatingUnitSet &operatingUnits() const override;
	OperatingUnitSet unitsProducing(const Material &material) const override;
	OperatingUnitSet unitsConsuming(const Material &material) const override;
	MaterialSet materialsProducedBy(const OperatingUnit &unit) const override;
	MaterialSet materialsConsumedBy(const OperatingUnit &unit) const override;
	OperatingUnitSet unitsProducingAnyOf(const MaterialSet &materials) const override;
	OperatingUnitSet unitsConsumingAnyOf(const MaterialSet &materials) const override;
	MaterialSet materialsProducedByAnyOf(const OperatingUnitSet &units) const override;
	MaterialSet materialsConsumedByAnyOf(const OperatingUnitSet &units) const override;

	std::string dumpData() const;
};

} // namespace PnsTools

#endif // ORIGINALPNSPROBLEM_H
