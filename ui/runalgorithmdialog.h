#ifndef RUNALGORITHMDIALOG_H
#define RUNALGORITHMDIALOG_H

#include <QDialog>
#include <QList>

namespace Ui {
class RunAlgorithmDialog;
}

class RunAlgorithmDialog : public QDialog
{
	Q_OBJECT

public:
	enum SelectedAction{
		ACTION_RUN,
		ACTION_CANCEL
	};

	enum SelectedAlgorithm{
		ALG_MSG,
		ALG_SSG,
		ALG_ABB
	};

	enum SelectedEvaluation{
		EVAL_NONE,
		EVAL_SUMWEIGHT,
		EVAL_MILP,
		EVAL_MINLP
	};

	enum AccelerationTechnique{
		ACCEL_RSG,
		ACCEL_NEUTRAL_EXTENSION
	};

	struct RunParameters
	{
		SelectedAlgorithm algorithm;
		SelectedEvaluation evaluation;
		int numberOfSolutions;
		int parallelProductionLimit;
		QList<AccelerationTechnique> accelerations;
	};


public:
	explicit RunAlgorithmDialog(QWidget *parent = nullptr);
	~RunAlgorithmDialog();

	QPair<SelectedAction,RunParameters> getRunParameters();

private slots:
	void on_radioButtonAlgMSG_clicked();
	void on_radioButtonAlgSSG_clicked();
	void on_radioButtonAlgABB_clicked();
	void on_pushButtonRun_clicked();
	void on_pushButtonCancel_clicked();

private:
	Ui::RunAlgorithmDialog *ui;
	SelectedAction lastAction=ACTION_CANCEL;
};

#endif // RUNALGORITHMDIALOG_H
