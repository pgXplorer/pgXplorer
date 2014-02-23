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

#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QtSql>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include "database.h"

class QSqlRecord;
class QSqlField;
class QSqlIndex;

class TableModel : public QSqlQueryModel
{
    Q_OBJECT

public slots:
    void clearCache();
    void setColumnAggregate(QStringList);
    void setCanUpdate(bool);

public:
    TableModel(Database *database, QStringList primary_key, QString table_name);

    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    bool setBulkData(const QModelIndexList &index_list, const QVariant &value);
    QVariant data(const QModelIndex & item, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool update();
    bool bulkUpdate();
    void setRowsFrom(int);
    int getPivotCol();
    int getPivotCat();
    int getPivotVal();
    void setPivotCol(int);
    void setPivotCat(int);
    void setPivotVal(int);

private:
    Database *database;
    QString table_name;
    int rows_from;
    QStringList primary_key;
    QStringList current_column_aggregates;
    bool primary_key_with_oid;
    bool can_update;
    QList<QStringList> primary_key_values_list;
    QStringList primary_key_values;
    QString edit_column;
    QModelIndex edit_index;
    QVariant edit_value;
    int pivot_col;
    int pivot_cat;
    int pivot_val;
    QMap<QModelIndex, QVariant> cache_values;

signals:
    void updateFailed(QString);
};

#endif // TABLEMODEL_H
