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

#include "tablemodel.h"

TableModel::TableModel(Database *database, QStringList primary_key, QString table_name)
{
    this->database = database;
    this->primary_key = primary_key;
    this->table_name = table_name;
}

Qt::ItemFlags TableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QSqlQueryModel::flags(index);
    flags |= Qt::ItemIsEditable;
    return flags;
}

bool TableModel::setData(const QModelIndex &index, const QVariant &value, int /* role */)
{
    //Make sure tables without primary key don't propagate changes.
    if(primary_key.isEmpty())
        return false;

    //Get the row data into a QSqlRecord object
    QSqlRecord rec = record(index.row());

    //Get parameters for UPDATE query
    edit_column = QString("\"" + rec.fieldName(index.column()) + "\"");
    edit_index = index;
    edit_value = value;

    //Check for primary keys to pass to UPDATE statment
    for(int column = 0; column < rec.count(); column++) {
        if(primary_key.contains(QString("\"" + rec.fieldName(column)) + "\""))
                primary_key_values.append(rec.value(column).toString());
    }

    //If UPDATE statement fails, do not store in cache
    //Clear the primary key values either case
    if(update()) {
        cache_values.insert(index, value);
        primary_key_values.clear();
        emit dataChanged(index, index);
        return true;
    }
    else {
        primary_key_values.clear();
        return false;
    }
}

QVariant TableModel::data(const QModelIndex &index, int role) const
{
    QModelIndex item = indexInQuery(index);
    //Align integers to the right
    if ((index.isValid() && role == Qt::TextAlignmentRole) && index.data().type() == QVariant::Int) {
        return Qt::AlignVCenter + Qt::AlignRight;
    }

    //Disable all roles except DisplayRole and EditRole
    if (!index.isValid() || (role != Qt::DisplayRole && role != Qt::EditRole))
        return QVariant();

    //Store the index into item to call the sibling of index.
    //QModelIndex item = indexInQuery(index);

    //Return cached values for UPDATEd data
    if(cache_values.contains(index)) {
        if(cache_values.value(index).type() == QVariant::Int && role == Qt::TextAlignmentRole)
            return Qt::AlignVCenter + Qt::AlignRight;
        return cache_values.value(index);
    }

    //Return sibling of index
    return QSqlQueryModel::data(index.sibling(item.row(), index.column()), role);
}

bool TableModel::update()
{
    //initialise the UPDATE status
    bool update_status = false;

    //Block to perform the actual UPDATE
    {
        QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", "update " + objectName());
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            emit updateFailed(tr("Unable to establish a database connection.\n"
                                 "No PostgreSQL support.\n"));
            return false;
        }

        QString query_string = QString("UPDATE ");
        query_string.append(table_name);
        query_string.append(QString(" SET "));
        query_string.append(edit_column);
        query_string.append(QString("=? WHERE "));
        query_string.append(primary_key.join("=? AND "));
        query_string.append(QString("=?"));

        QSqlQuery query(database_connection);
        query.prepare(query_string);
        query.addBindValue(edit_value.toString());
        foreach(QString primary_key_value, primary_key_values)
            query.addBindValue(primary_key_value);

        update_status = query.exec();
        if(update_status == false)
            emit updateFailed(query.lastError().text());
    }
    QSqlDatabase::removeDatabase("update " + objectName());
    return update_status;
}

void TableModel::clearCache()
{
    cache_values.clear();
}

