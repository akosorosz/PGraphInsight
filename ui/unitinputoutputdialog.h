#ifndef UNITINPUTOUTPUTDIALOG_H
#define UNITINPUTOUTPUTDIALOG_H

#include <QDialog>

namespace Ui {
class UnitInputOutputDialog;
}

class UnitInputOutputDialog : public QDialog
{
	Q_OBJECT
public:
	enum SelectedAction{
		ACTION_NEW,
		ACTION_SAVE,
		ACTION_DELETE,
		ACTION_CANCEL
	};

	struct InputOutputData
	{
		QString materialName;
		double flowRate;
	};

public:
	explicit UnitInputOutputDialog(QWidget *parent = nullptr, int unitId=-1, const QString &unitName="", bool isInputConnection=true);
	~UnitInputOutputDialog();

	static QPair<SelectedAction,InputOutputData> newIOData(QWidget *parent = nullptr, int unitId=-1, const QString &unitName="", bool isInput=true);
	static QPair<SelectedAction,InputOutputData> editIOData(QWidget *parent = nullptr, InputOutputData startingValues=InputOutputData(), int unitId=-1, const QString &unitName="", bool isInput=true);

private slots:
	void on_pushButtonAddNew_clicked();
	void on_pushButtonSave_clicked();
	void on_pushButtonDelete_clicked();
	void on_pushButtonCancel_clicked();

private:
	Ui::UnitInputOutputDialog *ui;
	SelectedAction lastAction=ACTION_CANCEL;
	InputOutputData originalData={QString(),1.0};
	QStringList existingMaterialNames, alreadyAddedMaterials;
	QString baseUnitName;
	int baseUnitId;
	bool isInput;
};

#endif // UNITINPUTOUTPUTDIALOG_H
