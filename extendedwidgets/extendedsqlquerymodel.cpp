#include "extendedsqlquerymodel.h"

ExtendedSqlQueryModel::ExtendedSqlQueryModel(QObject *parent)
	: QSqlQueryModel(parent)
{
}

QVariant ExtendedSqlQueryModel::data(const QModelIndex &index, int role) const
{
	QVariant originalData=QSqlQueryModel::data(index,role);
	if (role==Qt::DisplayRole && originalData.isNull()) return QString("-");
	else return originalData;
}
