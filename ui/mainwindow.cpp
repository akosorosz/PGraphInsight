#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStyle>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlRelation>
#include <QSqlRelationalDelegate>
#include <QDebug>
#include <QFileInfo>
#include <QDateTime>
#include "pns/algorithmmsg.h"
#include "pns/algorithmssg.h"
#include "pns/algorithmabb.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	this->setWindowTitle(applicationName);
	ui->tabWidget->setCurrentIndex(0);

	ui->actionNew->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
	ui->actionOpen->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
	ui->actionSaveAs->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
	ui->actionQuit->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
	ui->actionRunAlgorithm->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
	ui->actionDeleteRunHistory->setIcon(style()->standardIcon(QStyle::SP_BrowserStop));
	ui->actionRunAlgorithm->setEnabled(false);
	ui->actionDeleteRunHistory->setEnabled(false);
	ui->actionSaveAs->setEnabled(false);

	ui->widgetMaterialsEditor->setEnabled(false);
	ui->widgetUnitsEditor->setEnabled(false);
	ui->widgetInputEditor->setEnabled(false);
	ui->widgetOutputEditor->setEnabled(false);

	database=QSqlDatabase::addDatabase("QSQLITE");

	materialsTableModel=new QSqlRelationalTableModel(this,database);
	rawMaterialsModel=new ExtendedSqlQueryModel(this);
	productsModel=new ExtendedSqlQueryModel(this);
	operatingUnitsTableModel=new QSqlTableModel(this,database);

	unitInputsModel=new ExtendedSqlQueryModel(this);
	unitOutputsModel=new ExtendedSqlQueryModel(this);

	runHistoryTableModel=new ExtendedSqlTableModel(this,database);

	algorithmDialog=new RunAlgorithmDialog(this);

	databaseVersionUpdateCallbacks[3]=&MainWindow::databaseUpdateFromV3ToV4;
}

MainWindow::~MainWindow()
{
	delete ui;
}


void MainWindow::on_actionNew_triggered()
{
	// select file
	QString fileName=QFileDialog::getSaveFileName(this, "Select place and name of new project", QString(), fileTypeFilter);
	if (fileName.isEmpty()) return;
	currentFileName=fileName;

	QFileInfo fileInfo(fileName);
	this->setWindowTitle(applicationName+QString(" - ") + fileInfo.fileName());

	// close previous
	if (database.isOpen())
		database.close();

	// create default database
	database.setDatabaseName(fileName);
	if (!database.open())
	{
		QMessageBox::critical(this, "Database creation", "Cannot create database");
		return;
	}
	resetDatabase();

	// set up views
	setupTableViews();
	updateMaterialNameMap();
	updateRunHistoryView();
	ui->widgetMaterialsEditor->setEnabled(true);
	ui->widgetUnitsEditor->setEnabled(true);
	ui->widgetInputEditor->setEnabled(false);
	ui->widgetOutputEditor->setEnabled(false);
	ui->actionRunAlgorithm->setEnabled(true);
	ui->actionDeleteRunHistory->setEnabled(false);
	ui->actionSaveAs->setEnabled(true);
	turnEditionOnOff(true);
	pnsProblem.clear();
}

void MainWindow::on_actionOpen_triggered()
{
	// select file
	QString prevFileName=currentFileName;
	QString fileName=QFileDialog::getOpenFileName(this, "Select place and name of new project", QString(), fileTypeFilter);
	if (fileName.isEmpty()) return;

	bool fileOpened=openFile(fileName);
	if (!fileOpened)
	{
		openFile(prevFileName);
	}
}

void MainWindow::on_actionSaveAs_triggered()
{
	if (currentFileName.isEmpty()) return;
	QString newFileName=QFileDialog::getSaveFileName(this, "Select place and name of new project", QString(), fileTypeFilter);
	if (newFileName.isEmpty()) return;
	QFile file(currentFileName);
	file.copy(newFileName);
	QString prevFileName=currentFileName;
	bool fileOpened=openFile(newFileName);
	if (!fileOpened)
	{
		openFile(prevFileName);
	}
	else
	{
//		deleteRunHistory(); // Good question if needed or not
	}
}

void MainWindow::on_actionQuit_triggered()
{
	this->close();
}

void MainWindow::on_actionRunAlgorithm_triggered()
{
	if (currentFileName.isEmpty()) return;
	auto [action,runParameters]=algorithmDialog->getRunParameters();{}
	if (action==RunAlgorithmDialog::ACTION_RUN)
	{
		if (problemEditingEnabled)
		{
			auto result=QMessageBox::question(this, "Run algorithm first time", "Once you start runnning algorithms, you will be unable to change the problem data. Do you continue?");
			if (result==QMessageBox::No) return;
		}
		QDateTime runStartTime=QDateTime::currentDateTime();
		turnEditionOnOff(false);
		try
		{
			if (pnsProblem.isEmpty()) buildProblemFromDatabase();
//			auto problemData=pnsProblem.dumpData();
//			qDebug() << problemData.c_str();
			if (runParameters.algorithm==RunAlgorithmDialog::ALG_MSG)
			{
				PnsTools::AlgorithmMSG msg(pnsProblem);
				msg.run();
				PnsTools::OperatingUnitSet maximalStructure=msg.getUnitsOfMaximalStructure();
				const std::list<PnsTools::StepOfAlgorithm> &msgSteps=msg.getSteps();

				QSqlQuery query;
				database.transaction();

				if (!query.exec("INSERT INTO runHistory (timeStamp, algorithm, optimalWeight, optimalCost, structures, steps) VALUES('" + runStartTime.toString("yyyy-MM-dd HH:mm:ss") + "', 'MSG', NULL, NULL, " + (maximalStructure.empty()?"0":"1") + ", " + QString::number(msgSteps.size()) + ")"))
				{
					qDebug() << "ERROR runHistory: " << query.lastError();
				}
				int lastRunId=query.lastInsertId().toInt();

				for (const auto &step : msgSteps)
				{
					if (!query.exec("INSERT INTO stepsOfAlgorithm (runId, stepId, parentStepId, unitsIncluded, unitsExcluded, localBound, comment) VALUES(" + QString::number(lastRunId) + ", " + QString::number(step.stepId) + ", " + QString::number(step.parentStepId) + ", '" + QString::fromStdString(PnsTools::getUnitNamesString(step.includedUnits)) + "', '" + QString::fromStdString(PnsTools::getUnitNamesString(step.excludedUnits)) + "', " + QString::number(step.localBound) + ", '" + QString::fromStdString(step.comment) + "')"))
					{
						qDebug() << "ERROR stepsOfAlgorithm: " << query.lastError();
					}
				}

				if (maximalStructure.empty()) return;

				if (!query.exec("INSERT INTO resultStructures (runId, strNumber, totalWeight, totalCost, materialCost, unitInvestmentCost, unitOperatingCost) VALUES(" + QString::number(lastRunId) + ", 1, NULL, NULL, NULL, NULL, NULL)"))
				{
					qDebug() << "ERROR resultStructures: " << query.lastError();
				}
				int lastStructureId=query.lastInsertId().toInt();

				PnsTools::MaterialSet materialsInResult=msg.getMaterialsOfMaximalStructure();
				QList<int> includedMaterialIds;
				for (PnsTools::Material material : materialsInResult)
				{
					includedMaterialIds.append(material->id);
					if (!query.exec("INSERT INTO materialsInStructure (structureId, materialId, absoluteFlow, cost, price) VALUES(" + QString::number(lastStructureId) + ", " + QString::number(material->id) + ", NULL, NULL, NULL)"))
					{
						qDebug() << "ERROR materialsInStructure: " << query.lastError();
					}
				}
				QList<int> includedUnitIds;
				for (PnsTools::OperatingUnit unit: maximalStructure)
				{
					includedUnitIds.append(unit->id);
					if (!query.exec("INSERT INTO unitsInStructure (structureId, unitId, size, totalCost, investmentCost, operatingCost) VALUES(" + QString::number(lastStructureId) + ", " + QString::number(unit->id) + ", NULL, NULL, NULL, NULL)"))
					{
						qDebug() << "ERROR unitsInStructure: " << query.lastError();
					}
				}
				if (!query.exec("SELECT id, unitId FROM inputOutput"))
				{
					qDebug() << "ERROR inputOutput: " << query.lastError();
				}
				else
				{
					QSqlQuery ioAddQuery;
					while (query.next())
					{
						int ioId=query.value("id").toInt();
						int unitId=query.value("unitId").toInt();
						if (includedUnitIds.contains(unitId))
						{
							if (!ioAddQuery.exec("INSERT INTO inputOutputInStructure (structureId, ioId, flow) VALUES (" + QString::number(lastStructureId) + ", " + QString::number(ioId) + ", NULL)"))
							{
								qDebug() << "ERROR inputOutputInStructure: " << ioAddQuery.lastError();
							}
						}
					}
				}
				database.commit();
				updateRunHistoryView();
			}
			else if (runParameters.algorithm==RunAlgorithmDialog::ALG_SSG)
			{
				unsigned int accelerations=0;
				for (const auto &accel : runParameters.accelerations)
				{
					if (accel==RunAlgorithmDialog::ACCEL_NEUTRAL_EXTENSION) accelerations|=PnsTools::AlgorithmBase::ACCEL_NEUTRAL_EXTENSION;
				}
				PnsTools::AlgorithmSSG ssg(pnsProblem, accelerations, runParameters.parallelProductionLimit);
				ssg.run();
				const auto &solutionStructures=ssg.getSolutionStructures();
				const std::list<PnsTools::StepOfAlgorithm> &msgSteps=ssg.getSteps();

				QSqlQuery query;
				database.transaction();

				if (!query.exec("INSERT INTO runHistory (timeStamp, algorithm, optimalWeight, optimalCost, structures, steps) VALUES('" + runStartTime.toString("yyyy-MM-dd HH:mm:ss") + "', 'SSG', NULL, NULL, " + QString::number(solutionStructures.size()) + ", " + QString::number(msgSteps.size()) + ")"))
				{
					qDebug() << "ERROR runHistory: " << query.lastError();
				}
				int lastRunId=query.lastInsertId().toInt();

				for (const auto &step : msgSteps)
				{
					if (!query.exec("INSERT INTO stepsOfAlgorithm (runId, stepId, parentStepId, unitsIncluded, unitsExcluded, localBound, comment) VALUES(" + QString::number(lastRunId) + ", " + QString::number(step.stepId) + ", " + QString::number(step.parentStepId) + ", '" + QString::fromStdString(PnsTools::getUnitNamesString(step.includedUnits)) + "', '" + QString::fromStdString(PnsTools::getUnitNamesString(step.excludedUnits)) + "', " + QString::number(step.localBound) + ", '" + QString::fromStdString(step.comment) + "')"))
					{
						qDebug() << "ERROR stepsOfAlgorithm: " << query.lastError();
					}
				}

				int solutionStructureId=0;

				for (const auto &solution : solutionStructures)
				{
					solutionStructureId++;

					if (!query.exec("INSERT INTO resultStructures (runId, strNumber, totalWeight, totalCost, materialCost, unitInvestmentCost, unitOperatingCost) VALUES(" + QString::number(lastRunId) + ", " + QString::number(solutionStructureId) + ", NULL, NULL, NULL, NULL, NULL)"))
					{
						qDebug() << "ERROR resultStructures: " << query.lastError();
					}
					int lastStructureId=query.lastInsertId().toInt();

					PnsTools::MaterialSet materialsInResult=solution.second;
					QList<int> includedMaterialIds;
					for (PnsTools::Material material : materialsInResult)
					{
						includedMaterialIds.append(material->id);
						if (!query.exec("INSERT INTO materialsInStructure (structureId, materialId, absoluteFlow, cost, price) VALUES(" + QString::number(lastStructureId) + ", " + QString::number(material->id) + ", NULL, NULL, NULL)"))
						{
							qDebug() << "ERROR materialsInStructure: " << query.lastError();
						}
					}
					QList<int> includedUnitIds;
					PnsTools::OperatingUnitSet unitsInResult=solution.first;
					for (PnsTools::OperatingUnit unit: unitsInResult)
					{
						includedUnitIds.append(unit->id);
						if (!query.exec("INSERT INTO unitsInStructure (structureId, unitId, size, totalCost, investmentCost, operatingCost) VALUES(" + QString::number(lastStructureId) + ", " + QString::number(unit->id) + ", NULL, NULL, NULL, NULL)"))
						{
							qDebug() << "ERROR unitsInStructure: " << query.lastError();
						}
					}
					if (!query.exec("SELECT id, unitId FROM inputOutput"))
					{
						qDebug() << "ERROR inputOutput: " << query.lastError();
					}
					else
					{
						QSqlQuery ioAddQuery;
						while (query.next())
						{
							int ioId=query.value("id").toInt();
							int unitId=query.value("unitId").toInt();
							if (includedUnitIds.contains(unitId))
							{
								if (!ioAddQuery.exec("INSERT INTO inputOutputInStructure (structureId, ioId, flow) VALUES (" + QString::number(lastStructureId) + ", " + QString::number(ioId) + ", NULL)"))
								{
									qDebug() << "ERROR inputOutputInStructure: " << ioAddQuery.lastError();
								}
							}
						}
					}
				}
				database.commit();
				updateRunHistoryView();
			}
			else if (runParameters.algorithm==RunAlgorithmDialog::ALG_ABB)
			{
				unsigned int accelerations=0;
				for (const auto &accel : runParameters.accelerations)
				{
					if (accel==RunAlgorithmDialog::ACCEL_RSG) accelerations|=PnsTools::AlgorithmBase::ACCEL_RSG;
					else if (accel==RunAlgorithmDialog::ACCEL_NEUTRAL_EXTENSION) accelerations|=PnsTools::AlgorithmBase::ACCEL_NEUTRAL_EXTENSION;
				}
				PnsTools::AlgorithmABB abb(pnsProblem, runParameters.numberOfSolutions, PnsTools::AlgorithmBase::EvaluationType(runParameters.evaluation), accelerations, runParameters.parallelProductionLimit);
				abb.run();
				const auto &solutionStructures=abb.getSolutionStructures();
				const std::list<PnsTools::StepOfAlgorithm> &msgSteps=abb.getSteps();

				QSqlQuery query;
				database.transaction();

				QString optimalSolutionValue="NULL";
				if (solutionStructures.size()>0) optimalSolutionValue=QString::number(solutionStructures.front().objectiveValue);

				if (!query.exec("INSERT INTO runHistory (timeStamp, algorithm, optimalWeight, optimalCost, structures, steps) VALUES('" + runStartTime.toString("yyyy-MM-dd HH:mm:ss") + "', 'ABB', " + optimalSolutionValue + ", NULL, " + QString::number(solutionStructures.size()) + ", " + QString::number(msgSteps.size()) + ")"))
				{
					qDebug() << "ERROR runHistory: " << query.lastError();
				}
				int lastRunId=query.lastInsertId().toInt();

				for (const auto &step : msgSteps)
				{
					if (!query.exec("INSERT INTO stepsOfAlgorithm (runId, stepId, parentStepId, unitsIncluded, unitsExcluded, localBound, comment) VALUES(" + QString::number(lastRunId) + ", " + QString::number(step.stepId) + ", " + QString::number(step.parentStepId) + ", '" + QString::fromStdString(PnsTools::getUnitNamesString(step.includedUnits)) + "', '" + QString::fromStdString(PnsTools::getUnitNamesString(step.excludedUnits)) + "', " + QString::number(step.localBound) + ", '" + QString::fromStdString(step.comment) + "')"))
					{
						qDebug() << "ERROR stepsOfAlgorithm: " << query.lastError();
					}
				}

				int solutionStructureId=0;

				for (const auto &solution : solutionStructures)
				{
					solutionStructureId++;

					if (!query.exec("INSERT INTO resultStructures (runId, strNumber, totalWeight, totalCost, materialCost, unitInvestmentCost, unitOperatingCost) VALUES(" + QString::number(lastRunId) + ", " + QString::number(solutionStructureId) + ", " + QString::number(solution.objectiveValue) + ", NULL, NULL, NULL, NULL)"))
					{
						qDebug() << "ERROR resultStructures: " << query.lastError();
					}
					int lastStructureId=query.lastInsertId().toInt();

					PnsTools::MaterialSet materialsInResult=solution.materials;
					QList<int> includedMaterialIds;
					for (PnsTools::Material material : materialsInResult)
					{
						includedMaterialIds.append(material->id);
						if (!query.exec("INSERT INTO materialsInStructure (structureId, materialId, absoluteFlow, cost, price) VALUES(" + QString::number(lastStructureId) + ", " + QString::number(material->id) + ", NULL, NULL, NULL)"))
						{
							qDebug() << "ERROR materialsInStructure: " << query.lastError();
						}
					}
					QList<int> includedUnitIds;
					PnsTools::OperatingUnitSet unitsInResult=solution.units;
					for (PnsTools::OperatingUnit unit: unitsInResult)
					{
						includedUnitIds.append(unit->id);
						if (!query.exec("INSERT INTO unitsInStructure (structureId, unitId, size, totalCost, investmentCost, operatingCost) VALUES(" + QString::number(lastStructureId) + ", " + QString::number(unit->id) + ", NULL, NULL, NULL, NULL)"))
						{
							qDebug() << "ERROR unitsInStructure: " << query.lastError();
						}
					}
					if (!query.exec("SELECT id, unitId FROM inputOutput"))
					{
						qDebug() << "ERROR inputOutput: " << query.lastError();
					}
					else
					{
						QSqlQuery ioAddQuery;
						while (query.next())
						{
							int ioId=query.value("id").toInt();
							int unitId=query.value("unitId").toInt();
							if (includedUnitIds.contains(unitId))
							{
								if (!ioAddQuery.exec("INSERT INTO inputOutputInStructure (structureId, ioId, flow) VALUES (" + QString::number(lastStructureId) + ", " + QString::number(ioId) + ", NULL)"))
								{
									qDebug() << "ERROR inputOutputInStructure: " << ioAddQuery.lastError();
								}
							}
						}
					}
				}
				database.commit();
				updateRunHistoryView();
			}
		}
		catch (std::exception &e)
		{
			QMessageBox::critical(this, "Problem data error", QString("There is an error in the problem data. The file might be corrupted.\nError description:\n")+QString(e.what()));
		}
	}
}

void MainWindow::on_actionDeleteRunHistory_triggered()
{
	auto result=QMessageBox::question(this, "Delete run history?", "This will delete all run history for this project, and you will return into editing mode. Do you continue?");
	if (result==QMessageBox::Yes)
		deleteRunHistory();
}

void MainWindow::on_actionAbout_triggered()
{
	AboutDialog dialog(this);
	dialog.exec();
}


void MainWindow::on_actionLicense_triggered()
{
	LicenseDialog dialog(this);
	dialog.exec();
}

void MainWindow::on_tableViewOperatingUnits_currentIndexChanged(const QModelIndex &newIndex)
{
	if (!newIndex.isValid() || newIndex.row()==-1)
	{
		currentSelectedOperatingUnit={QString(),-1};
		ui->labelUnitInputs->setText(QString("Inputs"));
		ui->labelUnitOutputs->setText(QString("Outputs"));
		ui->widgetInputEditor->setEnabled(false);
		ui->widgetOutputEditor->setEnabled(false);
	}
	else
	{
		currentSelectedOperatingUnit={operatingUnitsTableModel->data(operatingUnitsTableModel->index(newIndex.row(),1)).toString(),
									 operatingUnitsTableModel->data(operatingUnitsTableModel->index(newIndex.row(),0)).toInt()};
		ui->labelUnitInputs->setText(QString("Inputs of ")+currentSelectedOperatingUnit.first);
		ui->labelUnitOutputs->setText(QString("Outputs of ")+currentSelectedOperatingUnit.first);
		if (problemEditingEnabled) ui->widgetInputEditor->setEnabled(true);
		if (problemEditingEnabled) ui->widgetOutputEditor->setEnabled(true);
	}
	updateInputOutputLists();
}


void MainWindow::on_pushButtonNewMaterial_clicked()
{
	if (problemEditingEnabled==false) return;
	auto [action,materialData] = MaterialDialog::newMaterial(this);{} // Qt creator STILL messes up indentation after structured bindings.
	if (action==MaterialDialog::ACTION_NEW)
	{
		QSqlQuery query(database);
		if (!query.exec("INSERT INTO materials (name,typeId,unitPrice,minFlow,maxFlow) VALUES ('" + materialData.name + "'," + QString::number(materialData.type) + "," + QString::number(materialData.unitPrice) + "," + QString::number(materialData.minFlow) + "," + QString::number(materialData.maxFlow) + ")"))
		{
			qDebug() << "ERROR materials: " << query.lastError();
		}
		materialsTableModel->select();
		updateMaterialGroupLists();
		updateMaterialNameMap();
	}
}

void MainWindow::on_pushButtonEditMaterial_clicked()
{
	if (problemEditingEnabled==false) return;
	on_tableViewMaterials_doubleClicked(ui->tableViewMaterials->currentIndex());
}

void MainWindow::on_tableViewMaterials_doubleClicked(const QModelIndex &index)
{
	if (problemEditingEnabled==false) return;
	MaterialDialog::MaterialData materialData;
	int currentRow=index.row();
	if (currentRow==-1) return; // Just in case
	materialData.name=materialsTableModel->data(materialsTableModel->index(currentRow,1)).toString();
	materialData.type=materialTypeFromText(materialsTableModel->data(materialsTableModel->index(currentRow,2)).toString());
	materialData.unitPrice=materialsTableModel->data(materialsTableModel->index(currentRow,3)).toDouble();
	materialData.minFlow=materialsTableModel->data(materialsTableModel->index(currentRow,4)).toDouble();
	materialData.maxFlow=materialsTableModel->data(materialsTableModel->index(currentRow,5)).toDouble();
	auto [action,newData] = MaterialDialog::editMaterial(this, materialData);{}
	if (action==MaterialDialog::ACTION_SAVE)
	{
		materialsTableModel->setData(materialsTableModel->index(currentRow,1), newData.name);
		materialsTableModel->setData(materialsTableModel->index(currentRow,2), static_cast<MaterialDialog::MaterialType>(newData.type));
		materialsTableModel->setData(materialsTableModel->index(currentRow,3), newData.unitPrice);
		materialsTableModel->setData(materialsTableModel->index(currentRow,4), newData.minFlow);
		materialsTableModel->setData(materialsTableModel->index(currentRow,5), newData.maxFlow);
		materialsTableModel->submitAll();
		updateMaterialGroupLists();
		updateMaterialNameMap();
		updateInputOutputLists();
	}
	else if (action==MaterialDialog::ACTION_DELETE)
	{
		int materialId=materialsTableModel->data(materialsTableModel->index(currentRow,0)).toInt();
		materialsTableModel->removeRow(currentRow);
		materialsTableModel->submitAll();
		materialsTableModel->select();
		QSqlQuery query(database);
		if (!query.exec("DELETE FROM inputOutput WHERE materialId="+QString::number(materialId)))
		{
			qDebug() << "ERROR inputOutput: " << query.lastError();
		}
		updateMaterialGroupLists();
		updateMaterialNameMap();
		updateInputOutputLists();
	}
}

void MainWindow::on_pushButtonDeleteMaterial_clicked()
{
	if (problemEditingEnabled==false) return;
	int currentRow=ui->tableViewMaterials->currentIndex().row();
	if (currentRow==-1) return; // Just in case
	auto result=QMessageBox::question(this, "Are you sure?", "This will remove the material. Are you sure?");
	if (result==QMessageBox::Yes)
	{
		int materialId=materialsTableModel->data(materialsTableModel->index(currentRow,0)).toInt();
		materialsTableModel->removeRow(currentRow);
		materialsTableModel->submitAll();
		materialsTableModel->select();
		QSqlQuery query(database);
		if (!query.exec("DELETE FROM inputOutput WHERE materialId="+QString::number(materialId)))
		{
			qDebug() << "ERROR inputOutput: " << query.lastError();
		}
		updateMaterialGroupLists();
		updateMaterialNameMap();
		updateInputOutputLists();
//		ui->tableViewMaterials->setCurrentIndex(QModelIndex());
	}
}


void MainWindow::on_pushButtonNewUnit_clicked()
{
	if (problemEditingEnabled==false) return;
	auto [action,unitData] = UnitDialog::newUnit(this);{} // Qt creator STILL messes up indentation after structured bindings.
	if (action==UnitDialog::ACTION_NEW)
	{
		QSqlQuery query(database);
		if (!query.exec("INSERT INTO units (name,weight,fixCapitalCost,propCapitalCost,fixOperatingCost,propOperatingCost,minSize,maxSize) VALUES ('"+unitData.name+"'," + QString::number(unitData.weight) + "," + QString::number(unitData.fixCapitalCost) + "," + QString::number(unitData.propCapitalCost) + "," + QString::number(unitData.fixOperatingCost) + "," + QString::number(unitData.propOperatingCost) + "," + QString::number(unitData.minSize) + "," + QString::number(unitData.maxSize) + ")"))
		{
			qDebug() << "ERROR units: " << query.lastError();
		}
		operatingUnitsTableModel->select();
	}
}

void MainWindow::on_pushButtonEditUnit_clicked()
{
	if (problemEditingEnabled==false) return;
	on_tableViewOperatingUnits_doubleClicked(ui->tableViewOperatingUnits->currentIndex());
}

void MainWindow::on_tableViewOperatingUnits_doubleClicked(const QModelIndex &index)
{
	if (problemEditingEnabled==false) return;
	UnitDialog::UnitData unitData;
	int currentRow=index.row();
	if (currentRow==-1) return; // Just in case
	unitData.name=operatingUnitsTableModel->data(operatingUnitsTableModel->index(currentRow,1)).toString();
	unitData.weight=operatingUnitsTableModel->data(operatingUnitsTableModel->index(currentRow,2)).toDouble();
	unitData.fixCapitalCost=operatingUnitsTableModel->data(operatingUnitsTableModel->index(currentRow,3)).toDouble();
	unitData.propCapitalCost=operatingUnitsTableModel->data(operatingUnitsTableModel->index(currentRow,4)).toDouble();
	unitData.fixOperatingCost=operatingUnitsTableModel->data(operatingUnitsTableModel->index(currentRow,5)).toDouble();
	unitData.propOperatingCost=operatingUnitsTableModel->data(operatingUnitsTableModel->index(currentRow,6)).toDouble();
	unitData.minSize=operatingUnitsTableModel->data(operatingUnitsTableModel->index(currentRow,7)).toDouble();
	unitData.maxSize=operatingUnitsTableModel->data(operatingUnitsTableModel->index(currentRow,8)).toDouble();
	auto [action,newData] = UnitDialog::editUnit(this, unitData);{}
	if (action==UnitDialog::ACTION_SAVE)
	{
		operatingUnitsTableModel->setData(operatingUnitsTableModel->index(currentRow,1), newData.name);
		operatingUnitsTableModel->setData(operatingUnitsTableModel->index(currentRow,2), newData.weight);
		operatingUnitsTableModel->setData(operatingUnitsTableModel->index(currentRow,3), newData.fixCapitalCost);
		operatingUnitsTableModel->setData(operatingUnitsTableModel->index(currentRow,4), newData.propCapitalCost);
		operatingUnitsTableModel->setData(operatingUnitsTableModel->index(currentRow,5), newData.fixOperatingCost);
		operatingUnitsTableModel->setData(operatingUnitsTableModel->index(currentRow,6), newData.propOperatingCost);
		operatingUnitsTableModel->setData(operatingUnitsTableModel->index(currentRow,7), newData.minSize);
		operatingUnitsTableModel->setData(operatingUnitsTableModel->index(currentRow,8), newData.maxSize);
		operatingUnitsTableModel->submitAll();
	}
	else if (action==UnitDialog::ACTION_DELETE)
	{
		int unitId=operatingUnitsTableModel->data(operatingUnitsTableModel->index(currentRow,0)).toInt();
		operatingUnitsTableModel->removeRow(currentRow);
		operatingUnitsTableModel->submitAll();
		operatingUnitsTableModel->select();
		QSqlQuery query(database);
		if (!query.exec("DELETE FROM inputOutput WHERE unitId="+QString::number(unitId)))
		{
			qDebug() << "ERROR inputOutput: " << query.lastError();
		}
		ui->tableViewOperatingUnits->setCurrentIndex(QModelIndex());
		on_tableViewOperatingUnits_currentIndexChanged(QModelIndex());
	}
}

void MainWindow::on_pushButtonDeleteUnit_clicked()
{
	if (problemEditingEnabled==false) return;
	int currentRow=ui->tableViewOperatingUnits->currentIndex().row();
	if (currentRow==-1) return; // Just in case
	auto result=QMessageBox::question(this, "Are you sure?", "This will remove the operating unit. Are you sure?");
	if (result==QMessageBox::Yes)
	{
		int unitId=operatingUnitsTableModel->data(operatingUnitsTableModel->index(currentRow,0)).toInt();
		operatingUnitsTableModel->removeRow(currentRow);
		operatingUnitsTableModel->submitAll();
		operatingUnitsTableModel->select();
		QSqlQuery query(database);
		if (!query.exec("DELETE FROM inputOutput WHERE unitId="+QString::number(unitId)))
		{
			qDebug() << "ERROR inputOutput: " << query.lastError();
		}
		ui->tableViewOperatingUnits->setCurrentIndex(QModelIndex());
		on_tableViewOperatingUnits_currentIndexChanged(QModelIndex());
	}
}


void MainWindow::on_pushButtonAddInput_clicked()
{
	if (problemEditingEnabled==false) return;
	if (currentSelectedOperatingUnit.second==-1) return;
	if (materialsTableModel->rowCount()==unitInputsModel->rowCount()) // Quick, but not too nice way of checking for free materials
	{
		QMessageBox::critical(this, "No more materials", "All materials are already added to this unit as an input");
		return;
	}
	auto [action,inputOutputData] = UnitInputOutputDialog::newIOData(this, currentSelectedOperatingUnit.second, currentSelectedOperatingUnit.first, true);{}
	if (action==UnitInputOutputDialog::ACTION_NEW)
	{
		QSqlQuery query(database);
		if (!query.exec("INSERT INTO inputOutput (unitId,materialId,isInput,flowRate) VALUES (" + QString::number(currentSelectedOperatingUnit.second) + "," + QString::number(materialNameToId[inputOutputData.materialName]) + ",1,"+QString::number(inputOutputData.flowRate)+")"))
		{
			qDebug() << "ERROR inputOutput: " << query.lastError();
		}
		updateInputOutputLists();
	}
}

void MainWindow::on_pushButtonEditInput_clicked()
{
	if (problemEditingEnabled==false) return;
	on_tableViewInputs_doubleClicked(ui->tableViewInputs->currentIndex());
}

void MainWindow::on_tableViewInputs_doubleClicked(const QModelIndex &index)
{
	if (problemEditingEnabled==false) return;
	if (currentSelectedOperatingUnit.second==-1) return;
	UnitInputOutputDialog::InputOutputData inputOutputData;
	int currentRow=index.row();
	if (currentRow==-1) return; // Just in case
	int currentIoId=unitInputsModel->data(unitInputsModel->index(currentRow,0)).toInt();
	inputOutputData.materialName=unitInputsModel->data(unitInputsModel->index(currentRow,1)).toString();
	inputOutputData.flowRate=unitInputsModel->data(unitInputsModel->index(currentRow,2)).toDouble();
	auto [action,newData] = UnitInputOutputDialog::editIOData(this, inputOutputData, currentSelectedOperatingUnit.second, currentSelectedOperatingUnit.first, true);{}
	if (action==UnitInputOutputDialog::ACTION_SAVE)
	{
		QSqlQuery query(database);
		if (!query.exec("UPDATE inputOutput SET materialId=" + QString::number(materialNameToId[newData.materialName]) + ", flowRate=" + QString::number(newData.flowRate) + " WHERE id="+QString::number(currentIoId)))
		{
			qDebug() << "ERROR inputOutput: " << query.lastError();
		}
		updateInputOutputLists();
	}
	else if (action==UnitInputOutputDialog::ACTION_DELETE)
	{
		QSqlQuery query(database);
		if (!query.exec("DELETE FROM inputOutput WHERE id="+QString::number(currentIoId)))
		{
			qDebug() << "ERROR inputOutput: " << query.lastError();
		}
		updateInputOutputLists();
	}
}

void MainWindow::on_pushButtonDeleteInput_clicked()
{
	if (problemEditingEnabled==false) return;
	if (currentSelectedOperatingUnit.second==-1) return;
	int currentRow=ui->tableViewInputs->currentIndex().row();
	if (currentRow==-1) return; // Just in case
	auto result=QMessageBox::question(this, "Are you sure?", "This will remove the input. Are you sure?");
	if (result==QMessageBox::Yes)
	{
		QSqlQuery query(database);
		int currentIoId=unitInputsModel->data(unitInputsModel->index(currentRow,0)).toInt();
		QString currentMaterialName=unitInputsModel->data(unitInputsModel->index(currentRow,1)).toString();
		if (!query.exec("DELETE FROM inputOutput WHERE id="+QString::number(currentIoId)))
		{
			qDebug() << "ERROR inputOutput: " << query.lastError();
		}
		updateInputOutputLists();
	}
}

void MainWindow::on_pushButtonAddOutput_clicked()
{
	if (problemEditingEnabled==false) return;
	if (currentSelectedOperatingUnit.second==-1) return;
	if (materialsTableModel->rowCount()==unitOutputsModel->rowCount()) // Quick, but not too nice way of checking for free materials
	{
		QMessageBox::critical(this, "No more materials", "All materials are already added to this unit as an output");
		return;
	}
	auto [action,inputOutputData] = UnitInputOutputDialog::newIOData(this, currentSelectedOperatingUnit.second, currentSelectedOperatingUnit.first, false);{}
	if (action==UnitInputOutputDialog::ACTION_NEW)
	{
		QSqlQuery query(database);
		if (!query.exec("INSERT INTO inputOutput (unitId,materialId,isInput,flowRate) VALUES (" + QString::number(currentSelectedOperatingUnit.second) + "," + QString::number(materialNameToId[inputOutputData.materialName]) + ",0,"+QString::number(inputOutputData.flowRate)+")"))
		{
			qDebug() << "ERROR inputOutput: " << query.lastError();
		}
		updateInputOutputLists();
	}
}

void MainWindow::on_pushButtonEditOutput_clicked()
{
	if (problemEditingEnabled==false) return;
	on_tableViewOutputs_doubleClicked(ui->tableViewOutputs->currentIndex());
}

void MainWindow::on_tableViewOutputs_doubleClicked(const QModelIndex &index)
{
	if (problemEditingEnabled==false) return;
	if (currentSelectedOperatingUnit.second==-1) return;
	UnitInputOutputDialog::InputOutputData inputOutputData;
	int currentRow=index.row();
	if (currentRow==-1) return; // Just in case
	int currentIoId=unitOutputsModel->data(unitOutputsModel->index(currentRow,0)).toInt();
	inputOutputData.materialName=unitOutputsModel->data(unitOutputsModel->index(currentRow,1)).toString();
	inputOutputData.flowRate=unitOutputsModel->data(unitOutputsModel->index(currentRow,2)).toDouble();
	auto [action,newData] = UnitInputOutputDialog::editIOData(this, inputOutputData, currentSelectedOperatingUnit.second, currentSelectedOperatingUnit.first, false);{}
	if (action==UnitInputOutputDialog::ACTION_SAVE)
	{
		QSqlQuery query(database);
		if (!query.exec("UPDATE inputOutput SET materialId=" + QString::number(materialNameToId[newData.materialName]) + ", flowRate=" + QString::number(newData.flowRate) + " WHERE id="+QString::number(currentIoId)))
		{
			qDebug() << "ERROR inputOutput: " << query.lastError();
		}
		updateInputOutputLists();
	}
	else if (action==UnitInputOutputDialog::ACTION_DELETE)
	{
		QSqlQuery query(database);
		if (!query.exec("DELETE FROM inputOutput WHERE id="+QString::number(currentIoId)))
		{
			qDebug() << "ERROR inputOutput: " << query.lastError();
		}
		updateInputOutputLists();
	}
}

void MainWindow::on_pushButtonDeleteOutput_clicked()
{
	if (problemEditingEnabled==false) return;
	if (currentSelectedOperatingUnit.second==-1) return;
	int currentRow=ui->tableViewOutputs->currentIndex().row();
	if (currentRow==-1) return; // Just in case
	auto result=QMessageBox::question(this, "Are you sure?", "This will remove the output. Are you sure?");
	if (result==QMessageBox::Yes)
	{
		QSqlQuery query(database);
		int currentIoId=unitOutputsModel->data(unitOutputsModel->index(currentRow,0)).toInt();
		QString currentMaterialName=unitOutputsModel->data(unitOutputsModel->index(currentRow,1)).toString();
		if (!query.exec("DELETE FROM inputOutput WHERE id="+QString::number(currentIoId)))
		{
			qDebug() << "ERROR inputOutput: " << query.lastError();
		}
		updateInputOutputLists();
	}
}

void MainWindow::on_tableViewRunHistory_doubleClicked(const QModelIndex &index)
{
	if (!index.isValid()) return;
	int currentRow=index.row();
	if (currentRow==-1) return;
	RunResultViewer resultView(runHistoryTableModel->data(runHistoryTableModel->index(currentRow,0)).toInt(), this);
	resultView.exec();
}

bool MainWindow::openFile(const QString &fileName)
{
	if (fileName.isEmpty())
	{
		currentFileName=fileName;
		this->setWindowTitle(applicationName);
		if (database.isOpen())
			database.close();
		ui->widgetMaterialsEditor->setEnabled(false);
		ui->widgetUnitsEditor->setEnabled(false);
		ui->widgetInputEditor->setEnabled(false);
		ui->widgetOutputEditor->setEnabled(false);
		ui->actionRunAlgorithm->setEnabled(false);
		ui->actionDeleteRunHistory->setEnabled(false);
		ui->actionSaveAs->setEnabled(false);
		materialsTableModel->clear();
		operatingUnitsTableModel->clear();
		unitInputsModel->clear();
		unitOutputsModel->clear();
		rawMaterialsModel->clear();
		productsModel->clear();
		turnEditionOnOff(false);
		pnsProblem.clear();
		return true;
	}
	currentFileName=fileName;

	QFileInfo fileInfo(fileName);
	this->setWindowTitle(applicationName+QString(" - ") + fileInfo.fileName());

	// close previous
	if (database.isOpen())
		database.close();

	// open
	database.setDatabaseName(fileName);
	if (!database.open())
	{
		QMessageBox::critical(this, "Opening database", "Cannot open database");
		return false;
	}

	int databaseVersion=0;
	QSqlQuery query(database);
	{
		if (query.exec("SELECT value FROM globalValues WHERE name='databaseVersion'"))
		{
			query.next();
			databaseVersion=query.value(0).toInt();
			query.finish();
		}
	}
	if (databaseVersion>currentDatabaseVersion)
	{
		QMessageBox::critical(this, "File format mismatch", "The format of the opened file seems to be newer than the current version of the software. Download the new version of the software");
		database.close();
		return false;
	}
	else if (databaseVersion<currentDatabaseVersion)
	{
		auto result=QMessageBox::warning(this, "File format mismatch", "The format of the opened file seems to be older than the current version of the software. If you open the file, the format will be updated to the current version. Do you continue?",QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
		if (result!=QMessageBox::Yes) return false;
		if (!updateDatabaseToCurrentVersion())
		{
			QMessageBox::critical(this, "Update failed", "Failed to update the file format");
			return false;
		}
	}

	// set up views
	setupTableViews();
	updateMaterialNameMap();
	ui->widgetMaterialsEditor->setEnabled(true);
	ui->widgetUnitsEditor->setEnabled(true);
	ui->widgetInputEditor->setEnabled(false);
	ui->widgetOutputEditor->setEnabled(false);
	ui->actionRunAlgorithm->setEnabled(true);
	ui->actionSaveAs->setEnabled(true);
	if (!query.exec("SELECT id FROM runHistory"))
	{
		qDebug() << "ERROR runHistory: " << query.lastError();
	}
	if (query.next())
	{
		turnEditionOnOff(false);
		ui->actionDeleteRunHistory->setEnabled(true);
		updateRunHistoryView();
	}
	else
	{
		turnEditionOnOff(true);
		ui->actionDeleteRunHistory->setEnabled(false);
		updateRunHistoryView();
	}
	pnsProblem.clear();
	return true;
}

void MainWindow::resetDatabase()
{
	database.transaction();
	// delete existing tables
	QSqlQuery query(database);
	for (QString tablename : database.tables())
	{
		query.exec("DROP TABLE " + tablename);
	}

	// create empty tables
	if (!query.exec("CREATE TABLE materials (id INTEGER PRIMARY KEY, name TEXT, typeId INTEGER, unitPrice REAL, minFlow REAL, maxFlow REAL)"))
	{
		qDebug() << "ERROR materials: " << query.lastError();
	}
	if (!query.exec("CREATE TABLE materialTypes (id INTEGER PRIMARY KEY, name TEXT)"))
	{
		qDebug() << "ERROR materialTypes: " << query.lastError();
	}
	if (!query.exec("CREATE TABLE units (id INTEGER PRIMARY KEY, name TEXT, weight REAL, fixCapitalCost REAL, propCapitalCost REAL, fixOperatingCost REAL, propOperatingCost REAL, minSize REAL, maxSize REAL)"))
	{
		qDebug() << "ERROR units: " << query.lastError();
	}
	if (!query.exec("CREATE TABLE inputOutput (id INTEGER PRIMARY KEY AUTOINCREMENT, unitId INTEGER, materialId INTEGER, isInput INTEGER(1), flowRate REAL)"))
	{
		qDebug() << "ERROR inputOutput: " << query.lastError();
	}
	if (!query.exec("CREATE TABLE globalValues (name TEXT, value TEXT)"))
	{
		qDebug() << "ERROR globalValues: " << query.lastError();
	}
	addRunHistoryTables();

	if (!query.exec("INSERT INTO materialTypes (id,name) VALUES (0,'Intermediate'),(1,'Raw material'),(2,'Product')"))
	{
		qDebug() << "ERROR materialsTypes: " << query.lastError();
	}
	if (!query.exec("INSERT INTO globalValues (name,value) VALUES ('databaseVersion','" + QString::number(currentDatabaseVersion) + "')"))
	{
		qDebug() << "ERROR globalValues: " << query.lastError();
	}
	database.commit();

}

void MainWindow::addRunHistoryTables()
{
	QSqlQuery query(database);
	if (!query.exec("CREATE TABLE runHistory (id INTEGER PRIMARY KEY AUTOINCREMENT, timeStamp TEXT, algorithm TEXT, optimalWeight REAL, optimalCost REAL, structures INTEGER, steps INTEGER)"))
	{
		qDebug() << "ERROR runHistory: " << query.lastError();
	}
	if (!query.exec("CREATE TABLE resultStructures (id INTEGER PRIMARY KEY AUTOINCREMENT, runId INTEGER, strNumber INTEGER, totalWeight REAL, totalCost REAL, materialCost REAL, unitInvestmentCost REAL, unitOperatingCost REAL)"))
	{
		qDebug() << "ERROR resultStructures: " << query.lastError();
	}
	if (!query.exec("CREATE TABLE unitsInStructure (id INTEGER PRIMARY KEY AUTOINCREMENT, structureId INTEGER, unitId INTEGER, size REAL, totalCost REAL, investmentCost REAL, operatingCost REAL)"))
	{
		qDebug() << "ERROR unitsInStructure: " << query.lastError();
	}
	if (!query.exec("CREATE TABLE materialsInStructure (id INTEGER PRIMARY KEY AUTOINCREMENT, structureId INTEGER, materialId INTEGER, absoluteFlow REAL, cost REAL, price REAL)"))
	{
		qDebug() << "ERROR materialsInStructure: " << query.lastError();
	}
	if (!query.exec("CREATE TABLE inputOutputInStructure (id INTEGER PRIMARY KEY AUTOINCREMENT, structureId INTEGER, ioId INTEGER, flow REAL)"))
	{
		qDebug() << "ERROR inputOutputInStructure: " << query.lastError();
	}
	if (!query.exec("CREATE TABLE stepsOfAlgorithm (id INTEGER PRIMARY KEY AUTOINCREMENT, runId INTEGER, stepId INTEGER, parentStepId INTEGER, unitsIncluded TEXT, unitsExcluded TEXT, localBound REAL, comment TEXT)"))
	{
		qDebug() << "ERROR stepsOfAlgorithm: " << query.lastError();
	}
}

bool MainWindow::updateDatabaseToCurrentVersion()
{
	int databaseVersion=0;
	{
		QSqlQuery query(database);
		if (query.exec("SELECT value FROM globalValues WHERE name='databaseVersion'"))
		{
			query.next();
			databaseVersion=query.value(0).toInt();
			query.finish();
		}
		else
		{
			return false;
		}
	}
	database.transaction();
	auto versionIt=databaseVersionUpdateCallbacks.find(databaseVersion);
	for (; versionIt!=databaseVersionUpdateCallbacks.end(); versionIt++)
	{
		if (!versionIt.value()(this))
		{
			database.rollback();
			return false;
		}
	}
	database.commit();

	return true;
}



void MainWindow::setupTableViews()
{
	materialsTableModel->setTable("materials");
	materialsTableModel->setEditStrategy(QSqlTableModel::OnFieldChange);
	materialsTableModel->setHeaderData(0, Qt::Horizontal, "Id #");
	materialsTableModel->setHeaderData(1, Qt::Horizontal, "Name");
	materialsTableModel->setHeaderData(2, Qt::Horizontal, "Type");
	materialsTableModel->setRelation(2, QSqlRelation("materialTypes", "id", "name"));
	materialsTableModel->setHeaderData(3, Qt::Horizontal, "Unit Price");
	materialsTableModel->setHeaderData(4, Qt::Horizontal, "Minimal flow");
	materialsTableModel->setHeaderData(5, Qt::Horizontal, "Maximal flow");
	materialsTableModel->select();
	ui->tableViewMaterials->setModel(materialsTableModel);
	ui->tableViewMaterials->hideColumn(0);

	updateMaterialGroupLists();

	operatingUnitsTableModel->setTable("units");
	operatingUnitsTableModel->setEditStrategy(QSqlTableModel::OnFieldChange);
	operatingUnitsTableModel->setHeaderData(0, Qt::Horizontal, "Id #");
	operatingUnitsTableModel->setHeaderData(1, Qt::Horizontal, "Name");
	operatingUnitsTableModel->setHeaderData(2, Qt::Horizontal, "Weight");
	operatingUnitsTableModel->setHeaderData(3, Qt::Horizontal, "Fix investment cost");
	operatingUnitsTableModel->setHeaderData(4, Qt::Horizontal, "Proportional investment cost");
	operatingUnitsTableModel->setHeaderData(5, Qt::Horizontal, "Fix operating cost");
	operatingUnitsTableModel->setHeaderData(6, Qt::Horizontal, "Proportional operating cost");
	operatingUnitsTableModel->setHeaderData(7, Qt::Horizontal, "Minimum capacity");
	operatingUnitsTableModel->setHeaderData(8, Qt::Horizontal, "Maximum capacity");
	operatingUnitsTableModel->select();
	ui->tableViewOperatingUnits->setModel(operatingUnitsTableModel);
	ui->tableViewOperatingUnits->hideColumn(0);
	ui->tableViewOperatingUnits->horizontalHeader()->setDefaultAlignment((Qt::Alignment)Qt::TextWordWrap);
	ui->tableViewOperatingUnits->horizontalHeader()->setFixedHeight(50);

	on_tableViewOperatingUnits_currentIndexChanged(QModelIndex());
}

void MainWindow::updateMaterialGroupLists()
{
	rawMaterialsModel->setQuery("SELECT name FROM materials WHERE typeId=1", database);
	ui->listViewRawMaterials->setModel(rawMaterialsModel);

	productsModel->setQuery("SELECT name FROM materials WHERE typeId=2", database);
	ui->listViewProducts->setModel(productsModel);
}

void MainWindow::updateInputOutputLists()
{
	if (currentSelectedOperatingUnit.second!=-1)
	{
		unitInputsModel->setQuery("SELECT io.id, mat.name AS 'Name', io.flowRate AS 'Flow rate' FROM inputOutput AS io INNER JOIN materials as mat ON io.materialId=mat.id WHERE io.unitId="+QString::number(currentSelectedOperatingUnit.second)+" AND io.isInput=1");
		ui->tableViewInputs->setModel(unitInputsModel);
		ui->tableViewInputs->hideColumn(0);


		unitOutputsModel->setQuery("SELECT io.id, mat.name AS 'Name', io.flowRate AS 'Flow rate' FROM inputOutput AS io INNER JOIN materials as mat ON io.materialId=mat.id WHERE io.unitId="+QString::number(currentSelectedOperatingUnit.second)+" AND io.isInput=0");
		ui->tableViewOutputs->setModel(unitOutputsModel);
		ui->tableViewOutputs->hideColumn(0);
	}
	else
	{
		unitInputsModel->clear();
		unitOutputsModel->clear();
	}
}

void MainWindow::updateMaterialNameMap()
{
	QSqlQuery query(database);
	if (!query.exec("SELECT id, name FROM materials"))
	{
		qDebug() << "ERROR materials: " << query.lastError();
		return;
	}
	materialNameToId.clear();
	while (query.next())
	{
		materialNameToId.insert(query.value(1).toString(), query.value(0).toInt());
	}
}

void MainWindow::updateRunHistoryView()
{
	runHistoryTableModel->setTable("runHistory");
	runHistoryTableModel->setHeaderData(0, Qt::Horizontal, "Id #");
	runHistoryTableModel->setHeaderData(1, Qt::Horizontal, "Started at");
	runHistoryTableModel->setHeaderData(2, Qt::Horizontal, "Algorithm");
	runHistoryTableModel->setHeaderData(3, Qt::Horizontal, "Optimal weight");
	runHistoryTableModel->setHeaderData(4, Qt::Horizontal, "Optimal cost");
	runHistoryTableModel->setHeaderData(5, Qt::Horizontal, "# of structures");
	runHistoryTableModel->setHeaderData(6, Qt::Horizontal, "# of recorded steps");
	runHistoryTableModel->select();
	ui->tableViewRunHistory->setModel(runHistoryTableModel);
	ui->tableViewRunHistory->hideColumn(0);
}

void MainWindow::turnEditionOnOff(bool turnOn)
{
	problemEditingEnabled=turnOn;
	if (currentFileName.isEmpty())
	{
		ui->widgetMaterialsEditor->setEnabled(false);
		ui->widgetUnitsEditor->setEnabled(false);
		ui->widgetInputEditor->setEnabled(false);
		ui->widgetOutputEditor->setEnabled(false);
		ui->actionDeleteRunHistory->setEnabled(false);
	}
	else
	{
		if (turnOn)
		{
			ui->widgetMaterialsEditor->setEnabled(true);
			ui->widgetUnitsEditor->setEnabled(true);
			if (ui->tableViewOperatingUnits->currentIndex().isValid())
			{
				ui->widgetInputEditor->setEnabled(true);
				ui->widgetOutputEditor->setEnabled(true);
			}
			else
			{
				ui->widgetInputEditor->setEnabled(false);
				ui->widgetOutputEditor->setEnabled(false);
			}
			ui->actionDeleteRunHistory->setEnabled(false);
		}
		else
		{
			ui->widgetMaterialsEditor->setEnabled(false);
			ui->widgetUnitsEditor->setEnabled(false);
			ui->widgetInputEditor->setEnabled(false);
			ui->widgetOutputEditor->setEnabled(false);
			ui->actionDeleteRunHistory->setEnabled(true);
		}
	}
}

void MainWindow::buildProblemFromDatabase()
{
	pnsProblem.clear();
	QSqlQuery query;
	if (!query.exec("SELECT * FROM materials"))
	{
		qDebug() << "ERROR materials: " << query.lastError();
	}
	while (query.next())
	{
		PnsTools::MaterialData newMaterial;
		newMaterial.id=query.value("id").toInt();
		newMaterial.name=query.value("name").toString().toStdString();
		newMaterial.type=PnsTools::MaterialType(query.value("typeId").toInt());
		newMaterial.unitPrice=query.value("unitPrice").toDouble();
		newMaterial.minFlow=query.value("minFlow").toDouble();
		newMaterial.maxFlow=query.value("maxFlow").toDouble();
		pnsProblem.addMaterialData(newMaterial);
	}
	if (!query.exec("SELECT * FROM units"))
	{
		qDebug() << "ERROR units: " << query.lastError();
	}
	while (query.next())
	{
		PnsTools::OperatingUnitData newUnit;
		newUnit.id=query.value("id").toInt();
		newUnit.name=query.value("name").toString().toStdString();
		newUnit.weight=query.value("weight").toString().toDouble();
		newUnit.fixCapitalCost=query.value("fixCapitalCost").toDouble();
		newUnit.propCapitalCost=query.value("propCapitalCost").toDouble();
		newUnit.fixOperatingCost=query.value("fixOperatingCost").toDouble();
		newUnit.propOperatingCost=query.value("propOperatingCost").toDouble();
		newUnit.minSize=query.value("minSize").toDouble();
		newUnit.maxSize=query.value("maxSize").toDouble();
		pnsProblem.addOperatingUnitData(newUnit);
	}
	if (!query.exec("SELECT * FROM inputOutput"))
	{
		qDebug() << "ERROR inputOutput: " << query.lastError();
	}
	while (query.next())
	{
		pnsProblem.addUnitMaterialConnection(query.value("unitId").toInt(),
											 query.value("materialId").toInt(),
											 query.value("isInput").toBool(),
											 query.value("flowRate").toDouble());
	}
}

void MainWindow::deleteRunHistory()
{
	ui->actionDeleteRunHistory->setEnabled(false);
	database.transaction();
	QSqlQuery query(database);

	if (!query.exec("DROP TABLE runHistory"))
	{
		qDebug() << "ERROR runHistory: " << query.lastError();
	}
	if (!query.exec("DROP TABLE resultStructures"))
	{
		qDebug() << "ERROR resultStructures: " << query.lastError();
	}
	if (!query.exec("DROP TABLE unitsInStructure"))
	{
		qDebug() << "ERROR unitsInStructure: " << query.lastError();
	}
	if (!query.exec("DROP TABLE materialsInStructure"))
	{
		qDebug() << "ERROR materialsInStructure: " << query.lastError();
	}
	if (!query.exec("DROP TABLE inputOutputInStructure"))
	{
		qDebug() << "ERROR inputOutputInStructure: " << query.lastError();
	}
	if (!query.exec("DROP TABLE stepsOfAlgorithm"))
	{
		qDebug() << "ERROR stepsOfAlgorithm: " << query.lastError();
	}
	addRunHistoryTables();
	database.commit();
	turnEditionOnOff(true);
	updateRunHistoryView();
	pnsProblem.clear();
}

MaterialDialog::MaterialType MainWindow::materialTypeFromText(const QString &type)
{
	if (type=="Intermediate") return MaterialDialog::INTERMEDIATE;
	else if (type=="Raw material") return MaterialDialog::RAW;
	else if (type=="Product") return MaterialDialog::PRODUCT;
	else return MaterialDialog::INTERMEDIATE;
}

bool MainWindow::databaseUpdateFromV3ToV4()
{
	QSqlQuery query(database);

	if (!query.exec("ALTER TABLE units RENAME TO unitsTemp3"))
	{
		qDebug() << "ERROR unitsTemp3 ALTER: " << query.lastError();
		return false;
	}
	if (!query.exec("CREATE TABLE units (id INTEGER PRIMARY KEY, name TEXT, weight REAL, fixCapitalCost REAL, propCapitalCost REAL, fixOperatingCost REAL, propOperatingCost REAL, minSize REAL, maxSize REAL)"))
	{
		qDebug() << "ERROR units CREATE: " << query.lastError();
		return false;
	}
	if (!query.exec("INSERT INTO units (id, name, weight, fixCapitalCost, propCapitalCost, fixOperatingCost, propOperatingCost, minSize, maxSize) SELECT id, name, 0, fixCapitalCost, propCapitalCost, fixOperatingCost, propOperatingCost, minSize, maxSize FROM unitsTemp3"))
	{
		qDebug() << "ERROR units INSERT: " << query.lastError();
		return false;
	}
	if (!query.exec("DROP TABLE unitsTemp3"))
	{
		qDebug() << "ERROR unitsTemp3 DROP: " << query.lastError();
		return false;
	}


	if (!query.exec("ALTER TABLE runHistory RENAME TO runHistoryTemp3"))
	{
		qDebug() << "ERROR runHistoryTemp3 ALTER: " << query.lastError();
		return false;
	}
	if (!query.exec("CREATE TABLE runHistory (id INTEGER PRIMARY KEY AUTOINCREMENT, timeStamp TEXT, algorithm TEXT, optimalWeight REAL, optimalCost REAL, structures INTEGER, steps INTEGER)"))
	{
		qDebug() << "ERROR runHistory CREATE: " << query.lastError();
		return false;
	}
	if (!query.exec("INSERT INTO runHistory (id, timeStamp, algorithm, optimalWeight, optimalCost, structures, steps) SELECT id, timeStamp, algorithm, NULL, optimalCost, structures, steps FROM runHistoryTemp3"))
	{
		qDebug() << "ERROR runHistory INSERT: " << query.lastError();
		return false;
	}
	if (!query.exec("DROP TABLE runHistoryTemp3"))
	{
		qDebug() << "ERROR runHistoryTemp3 DROP: " << query.lastError();
		return false;
	}


	if (!query.exec("ALTER TABLE resultStructures RENAME TO resultStructuresTemp3"))
	{
		qDebug() << "ERROR resultStructuresTemp3 ALTER: " << query.lastError();
		return false;
	}
	if (!query.exec("CREATE TABLE resultStructures (id INTEGER PRIMARY KEY AUTOINCREMENT, runId INTEGER, strNumber INTEGER, totalWeight REAL, totalCost REAL, materialCost REAL, unitInvestmentCost REAL, unitOperatingCost REAL)"))
	{
		qDebug() << "ERROR resultStructures CREATE: " << query.lastError();
		return false;
	}
	if (!query.exec("INSERT INTO resultStructures (id, runId, strNumber, totalWeight, totalCost, materialCost, unitInvestmentCost, unitOperatingCost) SELECT id, runId, strNumber, NULL, totalCost, materialCost, unitInvestmentCost, unitOperatingCost FROM resultStructuresTemp3"))
	{
		qDebug() << "ERROR resultStructures INSERT: " << query.lastError();
		return false;
	}
	if (!query.exec("DROP TABLE resultStructuresTemp3"))
	{
		qDebug() << "ERROR resultStructuresTemp3 DROP: " << query.lastError();
		return false;
	}

	if (!query.exec("UPDATE globalValues SET value=4 WHERE name='databaseVersion'"))
	{
		qDebug() << "ERROR globalValues: " << query.lastError();
		return false;
	}
	return true;
}

