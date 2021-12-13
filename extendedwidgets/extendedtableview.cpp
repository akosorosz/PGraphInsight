#include "extendedtableview.h"

void ExtendedTableView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
//	scrollTo(current);
	QTableView::currentChanged(current,previous);
	emit currentIndexChanged(current);
}
