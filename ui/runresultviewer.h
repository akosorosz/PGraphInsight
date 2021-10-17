#ifndef RUNRESULTVIEWER_H
#define RUNRESULTVIEWER_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include "structurebrowser.h"
#include "../extendedwidgets/extendedsqlquerymodel.h"

namespace Ui {
class RunResultViewer;
}

class RunResultViewer : public QDialog
{
	Q_OBJECT

public:
	explicit RunResultViewer(int runId, QWidget *parent = nullptr);
	~RunResultViewer();

private slots:
	void on_tableViewSteps_currentIndexChanged(const QModelIndex &newIndex);
	void on_tableViewStructures_doubleClicked(const QModelIndex &index);

private:
	Ui::RunResultViewer *ui;
	int runId;
	QSqlDatabase database;
	ExtendedSqlQueryModel *structuresModel;
	ExtendedSqlQueryModel *stepsModel;

	void fillStructureView();
	void fillStepView();
};

#endif // RUNRESULTVIEWER_H
