#include "originalpnsproblem.h"

namespace PnsTools {

void OriginalPnsProblem::clear()
{
	mMaterials.clear();
	mRawMaterials.clear();
	mProducts.clear();
	mIntermediates.clear();
	mOperatingUnits.clear();
	mMaterialDataList.clear();
	mOperatingUnitDataList.clear();
	mUnitsThatProduceMaterial.clear();
	mUnitsThatConsumeMaterial.clear();
	mMaterialsProducedByUnit.clear();
	mMaterialsConsumedByUnit.clear();
	mMaterialIdToPointer.clear();
	mOperatingUnitIdToPointer.clear();
	mInputFlowRates.clear();
	mOutputFlowRates.clear();
	mIsEmpty=true;
}

void OriginalPnsProblem::addMaterialData(const MaterialData &newMaterialData)
{
	if (mMaterialIdToPointer.find(newMaterialData.id)!=mMaterialIdToPointer.end())
		throw MaterialIdExistsException(newMaterialData.id);
	mMaterialDataList.push_back(newMaterialData);
	Material newMaterial=&(*mMaterialDataList.rbegin());
	mMaterials+=newMaterial;
	if (newMaterial->type==RAW) mRawMaterials+=newMaterial;
	else if (newMaterial->type==PRODUCT) mProducts+=newMaterial;
	else mIntermediates+=newMaterial;
	mMaterialIdToPointer.insert({newMaterial->id,newMaterial});
	mIsEmpty=false;
}

void OriginalPnsProblem::addOperatingUnitData(const OperatingUnitData &newOperatingUnitData)
{
	if (mOperatingUnitIdToPointer.find(newOperatingUnitData.id)!=mOperatingUnitIdToPointer.end())
		throw OperatingUnitIdExistsException(newOperatingUnitData.id);
	mOperatingUnitDataList.push_back(newOperatingUnitData);
	OperatingUnit newUnit=&(*mOperatingUnitDataList.rbegin());
	mOperatingUnits+=newUnit;
	mOperatingUnitIdToPointer.insert({newUnit->id,newUnit});
	mIsEmpty=false;
}

void OriginalPnsProblem::addUnitMaterialConnection(int unitId, int materialId, bool isInput, double flowRate)
{
	auto unitIter=mOperatingUnitIdToPointer.find(unitId);
	if (unitIter==mOperatingUnitIdToPointer.end()) throw OperatingUnitIdDoesNotExistException(unitId);
	auto materialIter=mMaterialIdToPointer.find(materialId);
	if (materialIter==mMaterialIdToPointer.end()) throw MaterialIdDoesNotExistException(materialId);
	OperatingUnit unit=unitIter->second;
	Material material=materialIter->second;
	if (isInput)
	{
		mUnitsThatConsumeMaterial[material]+=unit;
		mMaterialsConsumedByUnit[unit]+=material;
		mInputFlowRates[unit][material]=flowRate;
	}
	else
	{
		mUnitsThatProduceMaterial[material]+=unit;
		mMaterialsProducedByUnit[unit]+=material;
		mOutputFlowRates[unit][material]=flowRate;
	}
}

bool OriginalPnsProblem::isEmpty() const
{
	return mIsEmpty;
}

const MaterialSet &OriginalPnsProblem::materials() const
{
	return mMaterials;
}

const MaterialSet &OriginalPnsProblem::rawMaterials() const
{
	return mRawMaterials;
}

const MaterialSet &OriginalPnsProblem::products() const
{
	return mProducts;
}

const MaterialSet &OriginalPnsProblem::intermediates() const
{
	return mIntermediates;
}

const OperatingUnitSet &OriginalPnsProblem::operatingUnits() const
{
	return mOperatingUnits;
}

OperatingUnitSet OriginalPnsProblem::unitsProducing(const Material &material) const
{
	auto unitIter=mUnitsThatProduceMaterial.find(material);
	if (unitIter==mUnitsThatProduceMaterial.end()) return OperatingUnitSet();
	else return unitIter->second;
}

OperatingUnitSet OriginalPnsProblem::unitsConsuming(const Material &material) const
{
	auto unitIter=mUnitsThatConsumeMaterial.find(material);
	if (unitIter==mUnitsThatConsumeMaterial.end()) return OperatingUnitSet();
	else return unitIter->second;
}

MaterialSet OriginalPnsProblem::materialsProducedBy(const OperatingUnit &unit) const
{
	auto materialIter=mMaterialsProducedByUnit.find(unit);
	if (materialIter==mMaterialsProducedByUnit.end()) return MaterialSet();
	else return materialIter->second;
}

MaterialSet OriginalPnsProblem::materialsConsumedBy(const OperatingUnit &unit) const
{
	auto materialIter=mMaterialsConsumedByUnit.find(unit);
	if (materialIter==mMaterialsConsumedByUnit.end()) return MaterialSet();
	else return materialIter->second;
}

OperatingUnitSet OriginalPnsProblem::unitsProducingAnyOf(const MaterialSet &materials) const
{
	OperatingUnitSet units;
	for (Material material : materials)
	{
		auto unitIter=mUnitsThatProduceMaterial.find(material);
		if (unitIter!=mUnitsThatProduceMaterial.end())
			units += unitIter->second;
	}
	return units;
}

OperatingUnitSet OriginalPnsProblem::unitsConsumingAnyOf(const MaterialSet &materials) const
{
	OperatingUnitSet units;
	for (Material material : materials)
	{
		auto unitIter=mUnitsThatConsumeMaterial.find(material);
		if (unitIter!=mUnitsThatConsumeMaterial.end())
			units += unitIter->second;
	}
	return units;
}

MaterialSet OriginalPnsProblem::materialsProducedByAnyOf(const OperatingUnitSet &units) const
{
	MaterialSet materials;
	for (OperatingUnit unit : units)
	{
		auto materialIter=mMaterialsProducedByUnit.find(unit);
		if (materialIter!=mMaterialsProducedByUnit.end())
			materials += materialIter->second;
	}
	return materials;
}

MaterialSet OriginalPnsProblem::materialsConsumedByAnyOf(const OperatingUnitSet &units) const
{
	MaterialSet materials;
	for (OperatingUnit unit : units)
	{
		auto materialIter=mMaterialsConsumedByUnit.find(unit);
		if (materialIter!=mMaterialsConsumedByUnit.end())
			materials += materialIter->second;
	}
	return materials;
}

std::string OriginalPnsProblem::dumpData() const
{
	std::string data;
	data+="materials: "+getMaterialNamesString(mMaterials)+"\n";
	data+="raw materials: "+getMaterialNamesString(mRawMaterials)+"\n";
	data+="products: "+getMaterialNamesString(mProducts)+"\n";
	data+="intermediates: "+getMaterialNamesString(mIntermediates)+"\n";
	data+="units: "+getUnitNamesString(mOperatingUnits)+"\n";
	for (Material m : mMaterials)
	{
		data+="Material "+m->name+":\n";
		data+="\tProduced by: "+getUnitNamesString(this->unitsProducing(m))+"\n";
		data+="\tConsumed by: "+getUnitNamesString(this->unitsConsuming(m))+"\n";
	}
	for (OperatingUnit u : mOperatingUnits)
	{
		data+="Unit "+u->name+":\n";
		data+="\tConsumes: "+getMaterialNamesString(this->materialsConsumedBy(u))+"\n";
		data+="\tProduces: "+getMaterialNamesString(this->materialsProducedBy(u))+"\n";
	}
	return data;
}

} // namespace PnsTools
