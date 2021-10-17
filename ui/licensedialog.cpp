#include "licensedialog.h"
#include "ui_licensedialog.h"
#include <QFile>

LicenseDialog::LicenseDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::LicenseDialog)
{
	ui->setupUi(this);
	QFile file("://LICENSE");
	file.open(QIODevice::ReadOnly);
	ui->textBrowserLicense->setText(file.readAll());
}

LicenseDialog::~LicenseDialog()
{
	delete ui;
}
