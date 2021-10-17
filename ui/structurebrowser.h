#ifndef STRUCTUREBROWSER_H
#define STRUCTUREBROWSER_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include "../extendedwidgets/extendedsqlquerymodel.h"

namespace Ui {
class StructureBrowser;
}

class StructureBrowser : public QDialog
{
	Q_OBJECT

public:
	explicit StructureBrowser(int structureId, QWidget *parent = nullptr);
	~StructureBrowser();

private slots:
	void on_tableViewMaterials_currentIndexChanged(const QModelIndex &newIndex);
	void on_tableViewUnits_currentIndexChanged(const QModelIndex &newIndex);

private:
	Ui::StructureBrowser *ui;
	int structureId;
	QSqlDatabase database;
	ExtendedSqlQueryModel *materialsModel;
	ExtendedSqlQueryModel *unitsProducingModel;
	ExtendedSqlQueryModel *unitsConsumingModel;
	ExtendedSqlQueryModel *unitsModel;
	ExtendedSqlQueryModel *materialsConsumedByModel;
	ExtendedSqlQueryModel *materialsProducedByModel;

	void fillMaterialsView();
	void fillUnitsView();
};

#endif // STRUCTUREBROWSER_H
