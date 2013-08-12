/*
  LICENSE AND COPYRIGHT INFORMATION - Please read carefully.

  Copyright (c) 2010-2013, davyjones <dj@pgxplorer.com>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "querymodel.h"

QueryModel::QueryModel()
{
    this->rows_from = 1;

    pivot_col = -1;
    pivot_cat = -1;
    pivot_val = -1;
}

void QueryModel::setRowsFrom(int rows_from)
{
    this->rows_from = rows_from;
}

int QueryModel::getPivotCol()
{
    return this->pivot_col;
}

int QueryModel::getPivotCat()
{
    return this->pivot_cat;
}

int QueryModel::getPivotVal()
{
    return this->pivot_val;
}

void QueryModel::setPivotCol(int pivot_col)
{
    this->pivot_col = pivot_col;
}

void QueryModel::setPivotCat(int pivot_cat)
{
    this->pivot_cat = pivot_cat;
}

void QueryModel::setPivotVal(int pivot_val)
{
    this->pivot_val = pivot_val;
}

void QueryModel::setColumnAggregate(QStringList aggs)
{
    current_column_aggregates = aggs;
}

QVariant QueryModel::data(const QModelIndex &index, int role) const
{
    //Store the index into item to call the sibling of index.
    QModelIndex item = indexInQuery(index);

    //Align integers to the right
    if ((index.isValid() && role == Qt::TextAlignmentRole) && (index.data().type() != QMetaType::QString))
        return (Qt::AlignVCenter + Qt::AlignRight);

    if(index.isValid() && role == Qt::BackgroundRole && index.column() == pivot_col)
        return QColor(255, 0, 0, 100);

    if(index.isValid() && role == Qt::BackgroundRole && index.column() == pivot_cat)
        return QColor(0, 255, 0, 100);

    //Disable all roles except DisplayRole
    if (!index.isValid() || (role != Qt::DisplayRole))
        return QVariant();

    //Return sibling of index
    return QSqlQueryModel::data(index.sibling(item.row(), index.column()), role);
}

QVariant QueryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Vertical && role == Qt::DisplayRole)
        return QVariant(rows_from + section);
    else
        return QSqlQueryModel::headerData(section, orientation, role);
}
