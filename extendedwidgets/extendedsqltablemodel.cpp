#include "extendedsqltablemodel.h"

ExtendedSqlTableModel::ExtendedSqlTableModel(QObject *parent, const QSqlDatabase &db)
	: QSqlTableModel(parent,db)
{
}

QVariant ExtendedSqlTableModel::data(const QModelIndex &index, int role) const
{
	QVariant originalData=QSqlTableModel::data(index,role);
	if (role==Qt::DisplayRole && originalData.isNull()) return QString("-");
	else return originalData;
}
