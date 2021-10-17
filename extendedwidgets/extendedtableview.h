#ifndef EXTENDEDTABLEVIEW_H
#define EXTENDEDTABLEVIEW_H

#include <QTableView>
#include <QObject>
#include <QWidget>

class ExtendedTableView : public QTableView
{
	Q_OBJECT
public:
	using QTableView::QTableView;

signals:
	void currentIndexChanged(const QModelIndex &newIndex);

protected slots:
	void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;
};

#endif // EXTENDEDTABLEVIEW_H
