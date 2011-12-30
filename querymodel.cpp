/*
  LICENSE AND COPYRIGHT INFORMATION - Please read carefully.

  Copyright (c) 2011, davyjones <davyjones@github.com>

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

void QueryModel::fetchData(QueryModel *query_model, QString command, QStringList vars)
{
    emit busySignal();
    database_connection_id = command + vars.at(5);
    QTime ts;
    ts.start();
    QSqlDatabase::removeDatabase("queryview" + database_connection_id);
    QSqlDatabase database_connection;
    database_connection = QSqlDatabase::addDatabase("QPSQL", "queryview" + database_connection_id);
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
    query_model->setQuery(command, database_connection);
    fetchDataSignal(query_model, ts.elapsed(), query_model->rowCount(), query_model->columnCount());
}

void QueryModel::destroyQueryModel()
{
    QSqlDatabase::removeDatabase("queryview" + database_connection_id);
    delete this;
}
