#ifndef EXTENDEDSQLTABLEMODEL_H
#define EXTENDEDSQLTABLEMODEL_H

#include <QAbstractItemModel>
#include <QSqlTableModel>

class ExtendedSqlTableModel : public QSqlTableModel
{
	Q_OBJECT

public:
	explicit ExtendedSqlTableModel(QObject *parent = nullptr, const QSqlDatabase &db = QSqlDatabase());

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
};

#endif // EXTENDEDSQLTABLEMODEL_H
