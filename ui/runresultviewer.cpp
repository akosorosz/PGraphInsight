#include "runresultviewer.h"
#include "ui_runresultviewer.h"

RunResultViewer::RunResultViewer(int runId, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::RunResultViewer),
	runId(runId)
{
	ui->setupUi(this);
	ui->tabWidget->setCurrentIndex(0);

	database=QSqlDatabase::database();
	structuresModel=new ExtendedSqlQueryModel(this);
	stepsModel=new ExtendedSqlQueryModel(this);

	fillStructureView();
	fillStepView();
}

RunResultViewer::~RunResultViewer()
{
	delete ui;
}

void RunResultViewer::on_tableViewSteps_currentIndexChanged(const QModelIndex &newIndex)
{
	if (!newIndex.isValid() || newIndex.row()==-1)
	{
		ui->textBrowserStepComment->clear();
		ui->labelComment->setText("Comment for step ??:");
	}
	else
	{
		int currentRow=newIndex.row();
		int stepId=stepsModel->data(stepsModel->index(currentRow,1)).toInt();
		QString comment=stepsModel->data(stepsModel->index(currentRow,6)).toString();
		ui->labelComment->setText("Comment for step #" + QString::number(stepId) + ":");
		ui->textBrowserStepComment->setText(comment);
	}
}

void RunResultViewer::on_tableViewStructures_doubleClicked(const QModelIndex &index)
{
	int currentRow=index.row();
	if (currentRow==-1) return;
	int structureId=structuresModel->data(structuresModel->index(currentRow,0)).toInt();
	StructureBrowser browser(structureId, this);
	browser.exec();
}

void RunResultViewer::fillStructureView()
{
	structuresModel->setQuery("SELECT id, strNumber AS 'Structure #', totalWeight AS 'Total weight', totalCost AS 'Total cost', materialCost AS 'Material cost', unitInvestmentCost AS 'Unit investment cost', unitOperatingCost AS 'Unit operating cost' FROM resultStructures WHERE runId="+QString::number(runId));
	ui->tableViewStructures->setModel(structuresModel);
	ui->tableViewStructures->hideColumn(0);
}

void RunResultViewer::fillStepView()
{
	stepsModel->setQuery("SELECT id, stepId AS 'Step #', parentStepId AS 'Parent step #', unitsIncluded AS 'Included units', unitsExcluded AS 'Excluded units', localBound AS 'Bound', comment FROM stepsOfAlgorithm WHERE runId="+QString::number(runId));
	ui->tableViewSteps->setModel(stepsModel);
	ui->tableViewSteps->hideColumn(0);
	ui->tableViewSteps->hideColumn(6);
}

