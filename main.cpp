#include "ui/mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QDateTime>
#include <QTextStream>

void debugOutputLoggerToFile(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
	QString source;
	if (type==QtDebugMsg) source="Debug";
	else if (type==QtInfoMsg) source="Info";
	else if (type==QtWarningMsg) source="Warning";
	else if (type==QtCriticalMsg) source="Critical";
	else if (type==QtFatalMsg) source="Fatal";
	if (source.isEmpty()) return;
	QString msgText=source + " message at " + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") + " : " + msg + "\n";
	QFile errorLogFile("errorlog.txt");
	if (errorLogFile.open(QIODevice::Append | QIODevice::Text))
	{
		QTextStream outstream(&errorLogFile);
		outstream << msgText;
	}
}

int main(int argc, char *argv[])
{
	qInstallMessageHandler(debugOutputLoggerToFile);
	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	return a.exec();
}
