#include "structurebrowser.h"
#include "ui_structurebrowser.h"

StructureBrowser::StructureBrowser(int structureId, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::StructureBrowser),
	structureId(structureId)
{
	ui->setupUi(this);
	ui->tabWidget->setCurrentIndex(1);

	database=QSqlDatabase::database();
	materialsModel=new ExtendedSqlQueryModel(this);
	unitsProducingModel=new ExtendedSqlQueryModel(this);
	unitsConsumingModel=new ExtendedSqlQueryModel(this);
	unitsModel=new ExtendedSqlQueryModel(this);
	materialsConsumedByModel=new ExtendedSqlQueryModel(this);
	materialsProducedByModel=new ExtendedSqlQueryModel(this);

	fillMaterialsView();
	fillUnitsView();
}

StructureBrowser::~StructureBrowser()
{
	delete ui;
}

void StructureBrowser::on_tableViewMaterials_currentIndexChanged(const QModelIndex &newIndex)
{
	if (!newIndex.isValid() || newIndex.row()==-1)
	{
		unitsProducingModel->clear();
		unitsConsumingModel->clear();
		ui->labelUnitsProducing->setText("Units producing ??:");
		ui->labelUnitsConsuming->setText("Units consuming ??:");
	}
	else
	{
		int currentRow=newIndex.row();
		int materialId=materialsModel->data(materialsModel->index(currentRow,1)).toInt();
		QString materialName=materialsModel->data(materialsModel->index(currentRow,2)).toString();
		ui->labelUnitsProducing->setText("Units producing " + materialName + ":");
		ui->labelUnitsConsuming->setText("Units consuming " + materialName + ":");
		unitsProducingModel->setQuery("SELECT u.name AS 'Operating unit', ios.flow AS 'Total flow' FROM inputOutputInStructure AS ios INNER JOIN inputOutput AS io ON ios.ioId=io.id INNER JOIN units as u on io.unitId=u.id WHERE ios.structureId="+QString::number(structureId)+" AND io.materialId="+QString::number(materialId)+" AND io.isInput=0");
		ui->tableViewUnitsProducing->setModel(unitsProducingModel);
		unitsConsumingModel->setQuery("SELECT u.name AS 'Operating unit', ios.flow AS 'Total flow' FROM inputOutputInStructure AS ios INNER JOIN inputOutput AS io ON ios.ioId=io.id INNER JOIN units as u on io.unitId=u.id WHERE ios.structureId="+QString::number(structureId)+" AND io.materialId="+QString::number(materialId)+" AND io.isInput=1");
		ui->tableViewUnitsConsuming->setModel(unitsConsumingModel);
	}
}

void StructureBrowser::on_tableViewUnits_currentIndexChanged(const QModelIndex &newIndex)
{
	if (!newIndex.isValid() || newIndex.row()==-1)
	{
		materialsConsumedByModel->clear();
		materialsProducedByModel->clear();
		ui->labelMaterialsConsumed->setText("Materials consumed by ??:");
		ui->labelMaterialsProduced->setText("Materials produced by ??:");
	}
	else
	{
		int currentRow=newIndex.row();
		int unitId=unitsModel->data(unitsModel->index(currentRow,1)).toInt();
		QString unitName=unitsModel->data(unitsModel->index(currentRow,2)).toString();
		ui->labelMaterialsConsumed->setText("Materials consumed by " + unitName + ":");
		ui->labelMaterialsProduced->setText("Materials produced by " + unitName + ":");
		materialsConsumedByModel->setQuery("SELECT m.name as 'Material', ios.flow AS 'Total flow' FROM inputOutputInStructure AS ios INNER JOIN inputOutput AS io ON ios.ioId=io.id INNER JOIN materials AS m ON io.materialId=m.id WHERE ios.structureId="+QString::number(structureId)+" AND io.unitId="+QString::number(unitId)+" AND io.isInput=1");
		ui->tableViewMaterialsConsumed->setModel(materialsConsumedByModel);
		materialsProducedByModel->setQuery("SELECT m.name as 'Material', ios.flow AS 'Total flow' FROM inputOutputInStructure AS ios INNER JOIN inputOutput AS io ON ios.ioId=io.id INNER JOIN materials AS m ON io.materialId=m.id WHERE ios.structureId="+QString::number(structureId)+" AND io.unitId="+QString::number(unitId)+" AND io.isInput=0");
		ui->tableViewMaterialsProduced->setModel(materialsProducedByModel);
	}
}

void StructureBrowser::fillMaterialsView()
{
	materialsModel->setQuery("SELECT ms.id, ms.materialId, m.name AS 'Material', mt.name AS 'Type', ms.absoluteFlow AS 'Absolute flow', ms.cost AS 'Cost', ms.price AS 'Price' FROM materialsInStructure AS ms INNER JOIN materials AS m ON ms.materialId=m.id INNER JOIN materialTypes AS mt on m.typeId=mt.id WHERE structureId="+QString::number(structureId));
	ui->tableViewMaterials->setModel(materialsModel);
	ui->tableViewMaterials->hideColumn(0);
	ui->tableViewMaterials->hideColumn(1);
}

void StructureBrowser::fillUnitsView()
{
	unitsModel->setQuery("SELECT us.id, us.unitId, u.name AS 'Operating unit', us.size AS 'Size', us.totalCost AS 'Total cost', us.investmentCost AS 'Investment cost', us.operatingCost AS 'Operating cost' FROM unitsInStructure AS us INNER JOIN units AS u ON us.unitId=u.id WHERE structureId="+QString::number(structureId));
	ui->tableViewUnits->setModel(unitsModel);
	ui->tableViewUnits->hideColumn(0);
	ui->tableViewUnits->hideColumn(1);
}
