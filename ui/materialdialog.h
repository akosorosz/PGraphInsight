#ifndef MATERIALDIALOG_H
#define MATERIALDIALOG_H

#include <QDialog>

namespace Ui {
class MaterialDialog;
}

class MaterialDialog : public QDialog
{
	Q_OBJECT

public:
	enum SelectedAction{
		ACTION_NEW,
		ACTION_SAVE,
		ACTION_DELETE,
		ACTION_CANCEL
	};

	enum MaterialType{
		INTERMEDIATE=0,
		RAW,
		PRODUCT
	};

	struct MaterialData
	{
		QString name;
		MaterialType type;
		double unitPrice;
		double minFlow;
		double maxFlow;
	};

public:
	explicit MaterialDialog(QWidget *parent = nullptr);
	~MaterialDialog();

	static QPair<SelectedAction,MaterialData> newMaterial(QWidget *parent = nullptr);
	static QPair<SelectedAction,MaterialData> editMaterial(QWidget *parent = nullptr, MaterialData startingValues=MaterialData());

private slots:
	void on_pushButtonAddNew_clicked();
	void on_pushButtonSave_clicked();
	void on_pushButtonDelete_clicked();
	void on_pushButtonCancel_clicked();
	void on_comboBoxType_currentIndexChanged(int index);

private:
	Ui::MaterialDialog *ui;
	SelectedAction lastAction=ACTION_CANCEL;
	MaterialData originalData={QString(),INTERMEDIATE,0.0,0.0,10000000.0};
	QStringList existingMaterialNames;

	void enableWidgetsBasedOnMaterialTypeIndex(int index);
};

#endif // MATERIALDIALOG_H
