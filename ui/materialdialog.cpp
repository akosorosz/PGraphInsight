#include "materialdialog.h"
#include "ui_materialdialog.h"
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>

MaterialDialog::MaterialDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::MaterialDialog)
{
	ui->setupUi(this);
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
	enableWidgetsBasedOnMaterialTypeIndex(ui->comboBoxType->currentIndex());
}

MaterialDialog::~MaterialDialog()
{
	delete ui;
}

QPair<MaterialDialog::SelectedAction, MaterialDialog::MaterialData> MaterialDialog::newMaterial(QWidget *parent)
{
	MaterialDialog dialog(parent);
	dialog.ui->pushButtonAddNew->show();
	dialog.ui->pushButtonCancel->show();
	dialog.ui->pushButtonAddNew->setDefault(true);

	dialog.exec();
	QPair<SelectedAction,MaterialData> returnPair{dialog.lastAction,{QString(),INTERMEDIATE,0.0,0.0,10000000.0}};
	if (dialog.lastAction==ACTION_NEW)
	{
		returnPair.second.name=dialog.ui->lineEditName->text();
		returnPair.second.type=static_cast<MaterialDialog::MaterialType>(dialog.ui->comboBoxType->currentIndex());
		returnPair.second.unitPrice=dialog.ui->doubleSpinBoxUnitPrice->value();
		returnPair.second.minFlow=dialog.ui->doubleSpinBoxMinFlow->value();
		returnPair.second.maxFlow=dialog.ui->doubleSpinBoxMaxFlow->value();
	}
	return returnPair;
}

QPair<MaterialDialog::SelectedAction, MaterialDialog::MaterialData> MaterialDialog::editMaterial(QWidget *parent, MaterialData startingValues)
{
	MaterialDialog dialog(parent);
	dialog.originalData=startingValues;
	dialog.ui->lineEditName->setText(startingValues.name);
	dialog.ui->comboBoxType->setCurrentIndex(startingValues.type);
	dialog.ui->doubleSpinBoxUnitPrice->setValue(startingValues.unitPrice);
	dialog.ui->doubleSpinBoxMinFlow->setValue(startingValues.minFlow);
	dialog.ui->doubleSpinBoxMaxFlow->setValue(startingValues.maxFlow);
	dialog.ui->pushButtonCancel->show();
	dialog.ui->pushButtonSave->show();
	dialog.ui->pushButtonDelete->show();
	dialog.ui->pushButtonSave->setDefault(true);
	dialog.enableWidgetsBasedOnMaterialTypeIndex(dialog.ui->comboBoxType->currentIndex());

	dialog.exec();
	QPair<SelectedAction,MaterialData> returnPair{dialog.lastAction,startingValues};
	if (dialog.lastAction==ACTION_SAVE)
	{
		returnPair.second.name=dialog.ui->lineEditName->text();
		returnPair.second.type=static_cast<MaterialDialog::MaterialType>(dialog.ui->comboBoxType->currentIndex());
		returnPair.second.unitPrice=dialog.ui->doubleSpinBoxUnitPrice->value();
		returnPair.second.minFlow=dialog.ui->doubleSpinBoxMinFlow->value();
		returnPair.second.maxFlow=dialog.ui->doubleSpinBoxMaxFlow->value();
	}
	return returnPair;
}

void MaterialDialog::on_pushButtonAddNew_clicked()
{
	QString newName=ui->lineEditName->text();
	if (newName.isEmpty())
	{
		QMessageBox::critical(this, "Empty material name!", "Material name cannot be empty!");
		return;
	}
	if (existingMaterialNames.contains(newName))
	{
		QMessageBox::critical(this, "Existing name!", "A material with this name already exists!");
		return;
	}
	this->accept();
	lastAction=ACTION_NEW;
}


void MaterialDialog::on_pushButtonSave_clicked()
{
	QString newName=ui->lineEditName->text();
	if (newName.isEmpty())
	{
		QMessageBox::critical(this, "Empty material name!", "Material name cannot be empty!");
		return;
	}
	if (newName!=originalData.name && existingMaterialNames.contains(newName))
	{
		QMessageBox::critical(this, "Existing name!", "A material with this name already exists!");
		return;
	}
	this->accept();
	lastAction=ACTION_SAVE;
}


void MaterialDialog::on_pushButtonDelete_clicked()
{
	auto result=QMessageBox::question(this, "Are you sure?", "This will remove the material. Are you sure?");
	if (result==QMessageBox::Yes)
	{
		this->accept();
		lastAction=ACTION_DELETE;
	}
}

void MaterialDialog::on_pushButtonCancel_clicked()
{
	this->reject();
	lastAction=ACTION_CANCEL;
}

void MaterialDialog::on_comboBoxType_currentIndexChanged(int index)
{
	enableWidgetsBasedOnMaterialTypeIndex(index);
}

void MaterialDialog::enableWidgetsBasedOnMaterialTypeIndex(int index)
{
	if (index==0)
	{
		ui->doubleSpinBoxUnitPrice->setEnabled(false);
		ui->doubleSpinBoxMinFlow->setEnabled(false);
		ui->doubleSpinBoxMaxFlow->setEnabled(true);
	}
	else if (index==1)
	{
		ui->doubleSpinBoxUnitPrice->setEnabled(true);
		ui->doubleSpinBoxMinFlow->setEnabled(false);
		ui->doubleSpinBoxMaxFlow->setEnabled(true);
	}
	else if (index==2)
	{
		ui->doubleSpinBoxUnitPrice->setEnabled(false);
		ui->doubleSpinBoxMinFlow->setEnabled(true);
		ui->doubleSpinBoxMaxFlow->setEnabled(false);
	}
	else
	{
		ui->doubleSpinBoxUnitPrice->setEnabled(true);
		ui->doubleSpinBoxMinFlow->setEnabled(true);
		ui->doubleSpinBoxMaxFlow->setEnabled(true);
	}
}
