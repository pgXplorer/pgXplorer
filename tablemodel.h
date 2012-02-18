/*
  LICENSE AND COPYRIGHT INFORMATION - Please read carefully.

  Copyright (c) 2011-2012, davyjones <davyjones@github>

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
#include <QSqlDriver>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include "database.h"

class QSqlRecord;
class QSqlField;
class QSqlIndex;

class TableModel: public QSqlQueryModel
{
    Q_OBJECT

public slots:
    void clearCache();

public:
    TableModel(Database *database, QStringList primary_key, QString table_name);

     Qt::ItemFlags flags(const QModelIndex &index) const;
     bool setData(const QModelIndex &index, const QVariant &value, int role);
     QVariant data ( const QModelIndex & item, int role = Qt::DisplayRole ) const;
     bool update();
     //void clearCache();

private:
     Database *database;
     QString table_name;
     QStringList primary_key;
     QStringList primary_key_values;
     QString edit_column;
     QModelIndex edit_index;
     QVariant edit_value;
     QMap<QModelIndex, QVariant> cache_values;

Q_SIGNALS:
     void updateFailed(QString);
};

#endif // TABLEMODEL_H
