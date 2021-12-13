#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QSqlQueryModel>
#include <QAbstractItemDelegate>
#include <QMap>
#include <functional>

#include "unitdialog.h"
#include "materialdialog.h"
#include "unitinputoutputdialog.h"
#include "runalgorithmdialog.h"
#include "runresultviewer.h"
#include "licensedialog.h"
#include "aboutdialog.h"
#include "../pns/originalpnsproblem.h"
#include "../extendedwidgets/extendedsqlquerymodel.h"
#include "../extendedwidgets/extendedsqltablemodel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private slots:
	void on_actionNew_triggered();
	void on_actionOpen_triggered();
	void on_actionSaveAs_triggered();
	void on_actionQuit_triggered();
	void on_actionRunAlgorithm_triggered();
	void on_actionDeleteRunHistory_triggered();
	void on_actionAbout_triggered();
	void on_actionLicense_triggered();

	void on_tableViewOperatingUnits_currentIndexChanged(const QModelIndex &newIndex);

	void on_pushButtonNewMaterial_clicked();
	void on_pushButtonEditMaterial_clicked();
	void on_tableViewMaterials_doubleClicked(const QModelIndex &index);
	void on_pushButtonDeleteMaterial_clicked();

	void on_pushButtonNewUnit_clicked();
	void on_pushButtonEditUnit_clicked();
	void on_tableViewOperatingUnits_doubleClicked(const QModelIndex &index);
	void on_pushButtonDeleteUnit_clicked();

	void on_pushButtonAddInput_clicked();
	void on_pushButtonEditInput_clicked();
	void on_tableViewInputs_doubleClicked(const QModelIndex &index);
	void on_pushButtonDeleteInput_clicked();

	void on_pushButtonAddOutput_clicked();
	void on_pushButtonEditOutput_clicked();
	void on_tableViewOutputs_doubleClicked(const QModelIndex &index);
	void on_pushButtonDeleteOutput_clicked();

	void on_tableViewRunHistory_doubleClicked(const QModelIndex &index);

private:
	Ui::MainWindow *ui;
	QString applicationName=QString("P-Graph Insight");
	QString fileTypeFilter=QString("P-Graph Insight Project (*.pgip)");
	QString currentFileName;
	const int currentDatabaseVersion=4; // version 3 was the first published (beta) version
	bool problemEditingEnabled=false;

	QSqlDatabase database;
	QSqlRelationalTableModel *materialsTableModel;
	ExtendedSqlQueryModel *rawMaterialsModel;
	ExtendedSqlQueryModel *productsModel;
	QSqlTableModel *operatingUnitsTableModel;
	ExtendedSqlQueryModel *unitInputsModel;
	ExtendedSqlQueryModel *unitOutputsModel;
	ExtendedSqlTableModel *runHistoryTableModel;

	QPair<QString,int> currentSelectedOperatingUnit={QString(),-1};
	QMap<QString,int> materialNameToId;

	PnsTools::OriginalPnsProblem pnsProblem;

	RunAlgorithmDialog *algorithmDialog;

	bool openFile(const QString &fileName);
	void resetDatabase();
	void addRunHistoryTables();
	bool updateDatabaseToCurrentVersion();
	void setupTableViews();
	void updateMaterialGroupLists();
	void updateInputOutputLists();
	void updateMaterialNameMap();
	void updateRunHistoryView();
	void turnEditionOnOff(bool turnOn);
	void buildProblemFromDatabase();
	void deleteRunHistory();

	MaterialDialog::MaterialType materialTypeFromText(const QString &type);

	// Describes which function is called to update from a given version to the next one
	QMap<int,std::function<bool(MainWindow*)>> databaseVersionUpdateCallbacks;

	// database version updates
	bool databaseUpdateFromV3ToV4();
};
#endif // MAINWINDOW_H
