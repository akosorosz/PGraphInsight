#include "unitdialog.h"
#include "ui_unitdialog.h"
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>

UnitDialog::UnitDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::UnitDialog)
{
	ui->setupUi(this);
	ui->pushButtonAddNew->hide();
	ui->pushButtonSave->hide();
	ui->pushButtonDelete->hide();
	ui->pushButtonCancel->hide();


	QSqlDatabase database=QSqlDatabase::database();
	QSqlQuery query(database);
	query.exec("SELECT name FROM units");
	while (query.next())
	{
		existingUnitNames.append(query.value(0).toString());
	}
}

UnitDialog::~UnitDialog()
{
	delete ui;
}

QPair<UnitDialog::SelectedAction,UnitDialog::UnitData> UnitDialog::newUnit(QWidget *parent)
{
	UnitDialog dialog(parent);
	dialog.ui->pushButtonAddNew->show();
	dialog.ui->pushButtonCancel->show();
	dialog.ui->pushButtonAddNew->setDefault(true);

	dialog.exec();
	QPair<SelectedAction,UnitData> returnPair{dialog.lastAction,{QString(),0.0,0.0,0.0,0.0,0.0,0.0,10000000.0}};
	if (dialog.lastAction==ACTION_NEW)
	{
		returnPair.second.name=dialog.ui->lineEditName->text();
		returnPair.second.weight=dialog.ui->doubleSpinBoxWeight->value();
		returnPair.second.fixCapitalCost=dialog.ui->doubleSpinBoxFixCapitalCost->value();
		returnPair.second.propCapitalCost=dialog.ui->doubleSpinBoxPropCapitalCost->value();
		returnPair.second.fixOperatingCost=dialog.ui->doubleSpinBoxFixOperatingCost->value();
		returnPair.second.propOperatingCost=dialog.ui->doubleSpinBoxPropOperatingCost->value();
		returnPair.second.minSize=dialog.ui->doubleSpinBoxMinSize->value();
		returnPair.second.maxSize=dialog.ui->doubleSpinBoxMaxSize->value();
	}
	return returnPair;
}

QPair<UnitDialog::SelectedAction,UnitDialog::UnitData> UnitDialog::editUnit(QWidget *parent, UnitData startingValues)
{
	UnitDialog dialog(parent);
	dialog.originalData=startingValues;
	dialog.ui->lineEditName->setText(startingValues.name);
	dialog.ui->doubleSpinBoxWeight->setValue(startingValues.weight);
	dialog.ui->doubleSpinBoxFixCapitalCost->setValue(startingValues.fixCapitalCost);
	dialog.ui->doubleSpinBoxPropCapitalCost->setValue(startingValues.propCapitalCost);
	dialog.ui->doubleSpinBoxFixOperatingCost->setValue(startingValues.fixOperatingCost);
	dialog.ui->doubleSpinBoxPropOperatingCost->setValue(startingValues.propOperatingCost);
	dialog.ui->doubleSpinBoxMinSize->setValue(startingValues.minSize);
	dialog.ui->doubleSpinBoxMaxSize->setValue(startingValues.maxSize);
	dialog.ui->pushButtonCancel->show();
	dialog.ui->pushButtonSave->show();
	dialog.ui->pushButtonDelete->show();
	dialog.ui->pushButtonSave->setDefault(true);

	dialog.exec();
	QPair<SelectedAction,UnitData> returnPair{dialog.lastAction,startingValues};
	if (dialog.lastAction==ACTION_SAVE)
	{
		returnPair.second.name=dialog.ui->lineEditName->text();
		returnPair.second.weight=dialog.ui->doubleSpinBoxWeight->value();
		returnPair.second.fixCapitalCost=dialog.ui->doubleSpinBoxFixCapitalCost->value();
		returnPair.second.propCapitalCost=dialog.ui->doubleSpinBoxPropCapitalCost->value();
		returnPair.second.fixOperatingCost=dialog.ui->doubleSpinBoxFixOperatingCost->value();
		returnPair.second.propOperatingCost=dialog.ui->doubleSpinBoxPropOperatingCost->value();
		returnPair.second.minSize=dialog.ui->doubleSpinBoxMinSize->value();
		returnPair.second.maxSize=dialog.ui->doubleSpinBoxMaxSize->value();
	}
	return returnPair;
}

void UnitDialog::on_pushButtonAddNew_clicked()
{
	QString newName=ui->lineEditName->text();
	if (newName.isEmpty())
	{
		QMessageBox::critical(this, "Empty unit name!", "Operating unit name cannot be empty!");
		return;
	}
	if (existingUnitNames.contains(newName))
	{
		QMessageBox::critical(this, "Existing name!", "An operating unit with this name already exists!");
		return;
	}
	this->accept();
	lastAction=ACTION_NEW;
}


void UnitDialog::on_pushButtonSave_clicked()
{
	QString newName=ui->lineEditName->text();
	if (newName.isEmpty())
	{
		QMessageBox::critical(this, "Empty unit name!", "Operating unit name cannot be empty!");
		return;
	}
	if (newName!=originalData.name && existingUnitNames.contains(newName))
	{
		QMessageBox::critical(this, "Existing name!", "An operating unit with this name already exists!");
		return;
	}
	this->accept();
	lastAction=ACTION_SAVE;
}


void UnitDialog::on_pushButtonDelete_clicked()
{
	auto result=QMessageBox::question(this, "Are you sure?", "This will remove the operating unit. Are you sure?");
	if (result==QMessageBox::Yes)
	{
		this->accept();
		lastAction=ACTION_DELETE;
	}
}


void UnitDialog::on_pushButtonCancel_clicked()
{
	this->reject();
	lastAction=ACTION_CANCEL;
}

