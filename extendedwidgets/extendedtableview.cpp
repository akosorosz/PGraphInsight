#include "extendedtableview.h"

void ExtendedTableView::currentChanged(const QModelIndex &current, const QModelIndex &)
{
	emit currentIndexChanged(current);
}
