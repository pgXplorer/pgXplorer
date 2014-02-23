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

#include "tablemodel.h"

TableModel::TableModel(Database *database, QStringList primary_key, QString table_name)
{
    this->database = database;
    this->primary_key = primary_key;
    this->primary_key_with_oid = primary_key_with_oid;
    this->table_name = table_name;
    this->rows_from = 1;
    this->can_update = false;

    pivot_col = -1;
    pivot_cat = -1;
    pivot_val = -1;
}

void TableModel::setRowsFrom(int rows_from)
{
    this->rows_from = rows_from;
}

int TableModel::getPivotCol()
{
    return this->pivot_col;
}

int TableModel::getPivotCat()
{
    return this->pivot_cat;
}

int TableModel::getPivotVal()
{
    return this->pivot_val;
}

void TableModel::setPivotCol(int pivot_col)
{
    this->pivot_col = pivot_col;
}

void TableModel::setPivotCat(int pivot_cat)
{
    this->pivot_cat = pivot_cat;
}

void TableModel::setPivotVal(int pivot_val)
{
    this->pivot_val = pivot_val;
}

Qt::ItemFlags TableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QSqlQueryModel::flags(index);

    //Make table editable only when primary key exists.
    if(!primary_key.isEmpty() && can_update)
        flags |= Qt::ItemIsEditable;
    return flags;
}

void TableModel::setColumnAggregate(QStringList aggs)
{
    current_column_aggregates = aggs;
}

void TableModel::setCanUpdate(bool can_update)
{
    this->can_update = can_update;
}

bool TableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    //Make sure tables without primary key don't propagate changes.
    if(primary_key.isEmpty())
        return false;

    //if(!primary_key.isEmpty() && primary_key_with_oid && index.column()==0)
    //    return false;

    //Make sure that only changed values are UPDATEd.
    if(value.toString().compare(data(index).toString()) == 0)
        return false;

    //Get the row data into a QSqlRecord object
    QSqlRecord rec = record(index.row());

    //Get parameters for UPDATE query
    edit_column = QString("\"" + rec.fieldName(index.column()) + "\"");
    edit_index = index;
    edit_value = value;

    //Initialise primary_key_values and number of columns.
    primary_key_values.clear();
    int column_count = rec.count();

    //Check for primary keys to pass to UPDATE statment
    for(int column = 0; column < column_count; column++) {
        if(primary_key.contains(QString("\"" + rec.fieldName(column)) + "\""))
                primary_key_values.append(rec.value(column).toString());
    }

    //If UPDATE statement fails, do not store in cache
    //Clear the primary key values in either case
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

bool TableModel::setBulkData(const QModelIndexList &indices, const QVariant &value)
{
    //Make sure tables without primary key don't propagate changes.
    if(primary_key.isEmpty())
        return false;

    //if(!primary_key.isEmpty() && primary_key_with_oid && index.column()==0)
    //    return false;

    //Make sure that only changed values are UPDATEd.
    if(value.toString().compare(data(indices.at(0)).toString()) == 0)
        return false;

    //Get the row data into a QSqlRecord object
    QSqlRecord rec = record(indices.at(0).row());

    //Get parameters for UPDATE query
    edit_column = QString("\"" + rec.fieldName(indices.at(0).column()) + "\"");
    edit_index = indices.at(0);
    edit_value = value;

    int column_count = rec.count();

    //Check for primary keys to pass to UPDATE statment
    foreach(QModelIndex idx, indices) {
        QSqlRecord rec = record(idx.row());

        if(value.toString().compare(data(idx).toString()) == 0) {
            continue;
        }

        //Initialise primary_key_values and number of columns.
        primary_key_values.clear();

        for(int column = 0; column < column_count; column++) {
            if(primary_key.contains(QString("\"" + rec.fieldName(column)) + "\""))
                    primary_key_values.append(rec.value(column).toString());
        }
        primary_key_values_list.append(primary_key_values);
    }

    //If UPDATE statement fails, do not store in cache
    //Clear the primary key values in either case
    if(bulkUpdate()) {
        QModelIndex idx;
        foreach(idx, indices) {
            cache_values.insert(idx, value);
            emit dataChanged(idx, idx);
        }
        primary_key_values.clear();
        primary_key_values_list.clear();
        return true;
    }
    else {
        primary_key_values.clear();
        primary_key_values_list.clear();
        return false;
    }
}

QVariant TableModel::data(const QModelIndex &index, int role) const
{
    //Store the index into item to call the sibling of index.
    QModelIndex item = indexInQuery(index);

    //Align integers to the right.
    if ((index.isValid() && role == Qt::TextAlignmentRole) && index.data().type() != QMetaType::QString)
        return (Qt::AlignVCenter + Qt::AlignRight);

    if(index.isValid() && role == Qt::BackgroundRole && index.column() == pivot_col)
        return QColor(255, 0, 0, 100);

    if(index.isValid() && role == Qt::BackgroundRole && index.column() == pivot_cat)
        return QColor(0, 255, 0, 100);

    //Disable all roles except DisplayRole and EditRole
    if (!index.isValid() || (role != Qt::DisplayRole && role != Qt::EditRole))
        return QVariant();

    //Return cached values for UPDATEd data
    if(cache_values.contains(index))
        return cache_values.value(index);

    //Return sibling of index
    return QSqlQueryModel::data(index.sibling(item.row(), index.column()), role);
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Vertical && role == Qt::DisplayRole)
        return QVariant(rows_from + section);
    else {
        //return QSqlQueryModel::headerData(section, orientation, role);
        return QVariant();
    }
}

bool TableModel::update()
{
    //initialise the UPDATE status
    bool update_status = false;

    //Code-block to perform the actual UPDATE
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

        QString query_string = QLatin1String("UPDATE ");
        query_string.append(table_name);
        query_string.append(QLatin1String(" SET "));
        query_string.append(edit_column);
        query_string.append(QLatin1String("=? WHERE "));
        query_string.append(primary_key.join("=? AND "));
        query_string.append(QLatin1String("=?"));

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

bool TableModel::bulkUpdate()
{
    //initialise the UPDATE status
    bool update_status = false;

    //Code-block to perform the actual UPDATE
    {
        QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", "bulk update " + objectName());
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

        QString query_string = QLatin1String("UPDATE ");
        query_string.append(table_name);
        query_string.append(QLatin1String(" SET "));
        query_string.append(edit_column);
        query_string.append(QLatin1String("=? WHERE ("));
        query_string.append(primary_key.join(","));
        query_string.append(QLatin1String(") IN ("));

        foreach(primary_key_values, primary_key_values_list) {
            query_string.append(QLatin1String("("));
            foreach(QString primary_key_value, primary_key_values) {
                query_string.append(QLatin1String("?,"));;
            }
            query_string.remove(query_string.length()-1,1);
            query_string.append(QLatin1String("),"));
        }
        query_string.remove(query_string.length()-1,1);
        query_string.append(QLatin1String(")"));

        QSqlQuery query(database_connection);
        query.prepare(query_string);
        query.addBindValue(edit_value.toString());

        foreach(primary_key_values, primary_key_values_list) {
            foreach(QString primary_key_value, primary_key_values) {
                query.addBindValue(primary_key_value);
            }
        }
        update_status = query.exec();
        if(update_status == false)
            emit updateFailed(query.lastError().text());
    }
    QSqlDatabase::removeDatabase("bulk update " + objectName());
    return update_status;
}

void TableModel::clearCache()
{
    cache_values.clear();
}
