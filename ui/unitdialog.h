#ifndef UNITDIALOG_H
#define UNITDIALOG_H

#include <QDialog>

namespace Ui {
class UnitDialog;
}

class UnitDialog : public QDialog
{
	Q_OBJECT

public:
	enum SelectedAction{
		ACTION_NEW,
		ACTION_SAVE,
		ACTION_DELETE,
		ACTION_CANCEL
	};

	struct UnitData
	{
		QString name;
		double weight;
		double fixCapitalCost;
		double propCapitalCost;
		double fixOperatingCost;
		double propOperatingCost;
		double minSize;
		double maxSize;
	};

public:
	explicit UnitDialog(QWidget *parent = nullptr);
	~UnitDialog();

	static QPair<SelectedAction,UnitData> newUnit(QWidget *parent = nullptr);
	static QPair<SelectedAction,UnitData> editUnit(QWidget *parent = nullptr, UnitData startingValues=UnitData());

private slots:
	void on_pushButtonAddNew_clicked();
	void on_pushButtonSave_clicked();
	void on_pushButtonDelete_clicked();
	void on_pushButtonCancel_clicked();

private:
	Ui::UnitDialog *ui;
	SelectedAction lastAction=ACTION_CANCEL;
	UnitData originalData={QString(),0.0,0.0,0.0,0.0,0.0,0.0,10000000.0};
	QStringList existingUnitNames;
};

#endif // UNITDIALOG_H
