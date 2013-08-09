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

#ifndef QUERYMODEL_H
#define QUERYMODEL_H

#include <QtGui>
#include <QObject>
#include <QSqlQueryModel>
#include <QtConcurrent/QtConcurrent>

class QueryModel : public QSqlQueryModel
{
    Q_OBJECT

private:
    QString database_connection_id;
    QString query_name;
    int rows_from;
    QStringList current_column_aggregates;
    int pivot_col;
    int pivot_cat;
    int pivot_val;

public:
    explicit QueryModel();
    ~QueryModel(){}
    void fetchData(QString, QStringList);
    void setQueryName(QString query_name)
    {
        this->query_name = query_name;
    }
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    void setRowsFrom(int);
    int getPivotCol();
    int getPivotCat();
    int getPivotVal();
    void setPivotCol(int);
    void setPivotCat(int);
    void setPivotVal(int);

public slots:
    void setColumnAggregate(QStringList);
};

#endif // QUERYMODEL_H
