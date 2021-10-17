#include "runalgorithmdialog.h"
#include "ui_runalgorithmdialog.h"

RunAlgorithmDialog::RunAlgorithmDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::RunAlgorithmDialog)
{
	ui->setupUi(this);
	ui->widgetEval->setEnabled(false);
	ui->widgetSolnum->setEnabled(false);
	ui->widgetParprod->setEnabled(false);
	ui->widgetAccel->setEnabled(false);
}

RunAlgorithmDialog::~RunAlgorithmDialog()
{
	delete ui;
}

QPair<RunAlgorithmDialog::SelectedAction, RunAlgorithmDialog::RunParameters> RunAlgorithmDialog::getRunParameters()
{
	this->lastAction=ACTION_CANCEL;
	this->exec();
	QPair<SelectedAction, RunParameters> returnPair{this->lastAction,{ALG_MSG,EVAL_NONE,1,10000000,{}}};
	if (this->lastAction==ACTION_RUN){
		if (this->ui->radioButtonAlgMSG->isChecked()) returnPair.second.algorithm=ALG_MSG;
		else if (this->ui->radioButtonAlgSSG->isChecked()) returnPair.second.algorithm=ALG_SSG;
		else if (this->ui->radioButtonAlgABB->isChecked()) returnPair.second.algorithm=ALG_ABB;

		if (this->ui->radioButtonEvalNone->isChecked()) returnPair.second.evaluation=EVAL_NONE;
		else if (this->ui->radioButtonEvalSumWeights->isChecked()) returnPair.second.evaluation=EVAL_SUMWEIGHT;
		else if (this->ui->radioButtonEvalMILP->isChecked()) returnPair.second.evaluation=EVAL_MILP;
		else if (this->ui->radioButtonEvalMINLP->isChecked()) returnPair.second.evaluation=EVAL_MINLP;

		if (this->ui->radioButtonSolnumBest->isChecked()) returnPair.second.numberOfSolutions=1;
		else if (this->ui->radioButtonSolnumNbest->isChecked()) returnPair.second.numberOfSolutions=this->ui->spinBoxSolnum->value();
		else if (this->ui->radioButtonSolnumAll->isChecked()) returnPair.second.numberOfSolutions=10000000;

		if (this->ui->radioButtonParprodLimit->isChecked()) returnPair.second.parallelProductionLimit=this->ui->spinBoxParprod->value();
		else if (this->ui->radioButtonParprodNoLimit->isChecked()) returnPair.second.parallelProductionLimit=10000000;

		if (this->ui->checkBoxAccelRSG->isChecked()) returnPair.second.accelerations.append(ACCEL_RSG);
		if (this->ui->checkBoxAccelNeutralExtension->isChecked()) returnPair.second.accelerations.append(ACCEL_NEUTRAL_EXTENSION);
	}
	return returnPair;
}

void RunAlgorithmDialog::on_radioButtonAlgMSG_clicked()
{
	ui->widgetEval->setEnabled(false);
	ui->widgetSolnum->setEnabled(false);
	ui->widgetParprod->setEnabled(false);
	ui->widgetAccel->setEnabled(false);
}


void RunAlgorithmDialog::on_radioButtonAlgSSG_clicked()
{
	ui->widgetEval->setEnabled(false);
	ui->widgetSolnum->setEnabled(false);
	ui->widgetParprod->setEnabled(true);
	ui->widgetAccel->setEnabled(true);
}


void RunAlgorithmDialog::on_radioButtonAlgABB_clicked()
{
	ui->widgetEval->setEnabled(true);
	ui->widgetSolnum->setEnabled(true);
	ui->widgetParprod->setEnabled(true);
	ui->widgetAccel->setEnabled(true);
}

void RunAlgorithmDialog::on_pushButtonRun_clicked()
{
	this->accept();
	lastAction=ACTION_RUN;
}

void RunAlgorithmDialog::on_pushButtonCancel_clicked()
{
	this->reject();
	lastAction=ACTION_CANCEL;
}

