#include "unitinputoutputdialog.h"
#include "ui_unitinputoutputdialog.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QSqlError>

UnitInputOutputDialog::UnitInputOutputDialog(QWidget *parent, int unitId, const QString &unitName, bool isInputConnection) :
	QDialog(parent),
	ui(new Ui::UnitInputOutputDialog),
	baseUnitName(unitName),
	baseUnitId(unitId),
	isInput(isInputConnection)
{
	ui->setupUi(this);
	if (isInput) ui->labelTitle->setText(QString("Input of operating unit ")+unitName);
	else ui->labelTitle->setText(QString("Output of operating unit ")+unitName);

	ui->pushButtonAddNew->hide();
	ui->pushButtonSave->hide();
	ui->pushButtonDelete->hide();
	ui->pushButtonCancel->hide();


	QSqlDatabase database=QSqlDatabase::database();
	QSqlQuery query(database);
	query.exec("SELECT name FROM materials");
	while (query.next())
	{
		existingMaterialNames.append(query.value(0).toString());
	}
	if (!query.exec("SELECT mat.name FROM inputOutput AS io INNER JOIN materials AS mat ON io.materialId=mat.id WHERE io.unitId="+QString::number(baseUnitId)+" and io.isInput="+(isInput?"1":"0")))
	{
		qDebug() << "ERROR inputOutput: " << query.lastError();
	}
	else
	{
		while (query.next())
		{
			alreadyAddedMaterials.append(query.value(0).toString());
		}
		ui->comboBoxMaterial->clear();
		for (QString matName : existingMaterialNames)
		{
			if (!alreadyAddedMaterials.contains(matName))
				ui->comboBoxMaterial->addItem(matName);
		}
	}
}

UnitInputOutputDialog::~UnitInputOutputDialog()
{
	delete ui;
}

QPair<UnitInputOutputDialog::SelectedAction, UnitInputOutputDialog::InputOutputData> UnitInputOutputDialog::newIOData(QWidget *parent, int unitId, const QString &unitName, bool isInput)
{
	UnitInputOutputDialog dialog(parent, unitId, unitName, isInput);
	dialog.ui->pushButtonAddNew->show();
	dialog.ui->pushButtonCancel->show();
	dialog.ui->pushButtonAddNew->setDefault(true);

	dialog.exec();
	QPair<SelectedAction,InputOutputData> returnPair{dialog.lastAction,{QString(),1.0}};
	if (dialog.lastAction==ACTION_NEW)
	{
		returnPair.second.materialName=dialog.ui->comboBoxMaterial->currentText();
		returnPair.second.flowRate=dialog.ui->doubleSpinBoxFlowRate->value();
	}
	return returnPair;
}

QPair<UnitInputOutputDialog::SelectedAction, UnitInputOutputDialog::InputOutputData> UnitInputOutputDialog::editIOData(QWidget *parent, InputOutputData startingValues, int unitId, const QString &unitName, bool isInput)
{
	UnitInputOutputDialog dialog(parent, unitId, unitName, isInput);
	dialog.originalData=startingValues;
//	dialog.ui->comboBoxMaterial->setCurrentText(startingValues.materialName);
	dialog.ui->comboBoxMaterial->insertItem(0, startingValues.materialName);
	dialog.ui->comboBoxMaterial->setCurrentIndex(0);
	dialog.ui->doubleSpinBoxFlowRate->setValue(startingValues.flowRate);
	dialog.ui->pushButtonCancel->show();
	dialog.ui->pushButtonSave->show();
	dialog.ui->pushButtonDelete->show();
	dialog.ui->pushButtonSave->setDefault(true);

	dialog.exec();
	QPair<SelectedAction,InputOutputData> returnPair{dialog.lastAction,startingValues};
	if (dialog.lastAction==ACTION_SAVE)
	{
		returnPair.second.materialName=dialog.ui->comboBoxMaterial->currentText();
		returnPair.second.flowRate=dialog.ui->doubleSpinBoxFlowRate->value();
	}
	return returnPair;
}

void UnitInputOutputDialog::on_pushButtonAddNew_clicked()
{
	this->accept();
	lastAction=ACTION_NEW;
}


void UnitInputOutputDialog::on_pushButtonSave_clicked()
{
	this->accept();
	lastAction=ACTION_SAVE;
}


void UnitInputOutputDialog::on_pushButtonDelete_clicked()
{
	auto result=QMessageBox::question(this, "Are you sure?", QString("This will remove the ")+(isInput?QString("input"):QString("output"))+QString(". Are you sure?"));
	if (result==QMessageBox::Yes)
	{
		this->accept();
		lastAction=ACTION_DELETE;
	}
}


void UnitInputOutputDialog::on_pushButtonCancel_clicked()
{
	this->reject();
	lastAction=ACTION_CANCEL;
}

