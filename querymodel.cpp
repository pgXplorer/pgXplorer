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

#include "querymodel.h"

QueryModel::QueryModel()
{
    this->rows_from = 1;
}

void QueryModel::fetchData(QString command, QStringList vars)
{
    emit busySignal();
    database_connection_id = command + vars.at(5);
    QTime ts;
    ts.start();
    {
        QSqlDatabase::removeDatabase("queryview " + database_connection_id);
        QSqlDatabase database_connection;
        database_connection = QSqlDatabase::addDatabase("QPSQL", "queryview " + database_connection_id);
        database_connection.setHostName(vars.at(0));
        database_connection.setPort(vars.at(1).toInt());
        database_connection.setDatabaseName(vars.at(2));
        database_connection.setUserName(vars.at(3));
        database_connection.setPassword(vars.at(4));
        if (!database_connection.open()) {
            qDebug() << tr("Couldn't connect to database.\n"
                           "Check connection parameters.\n");
            return;
        }
        setQuery(command, database_connection);
    }
    QSqlDatabase::removeDatabase("queryview " + database_connection_id);
    fetchDataSignal(ts.elapsed(), rowCount(), columnCount());
}

void QueryModel::setRowsFrom(int rows_from)
{
    this->rows_from = rows_from;
}

QVariant QueryModel::data(const QModelIndex &index, int role) const
{
    //Store the index into item to call the sibling of index.
    QModelIndex item = indexInQuery(index);

    //Align integers to the right
    if ((index.isValid() && role == Qt::TextAlignmentRole) && (index.data().type() != QMetaType::QString))
        return (Qt::AlignVCenter + Qt::AlignRight);

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
