#ifndef EXTENDEDSQLQUERYMODEL_H
#define EXTENDEDSQLQUERYMODEL_H

#include <QSqlQueryModel>

class ExtendedSqlQueryModel : public QSqlQueryModel
{
	Q_OBJECT

public:
	explicit ExtendedSqlQueryModel(QObject *parent = nullptr);

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
};

#endif // EXTENDEDSQLQUERYMODEL_H
