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

#include "database.h"
#include "connectionproperties.h"

ulong Database::dbViewObjectId = 0;

Database::Database(MainWin *mainwin, int database_id)
{
    if(database_id == 0)
        this->database_id = ++dbViewObjectId;
    else
        this->database_id = database_id;
    this->mainwin = mainwin;
    setDatabaseStatus(false);
    setParamStatus(false);
    setDatabaseCollapsed(true);
    setFlags(QGraphicsItem::ItemIsSelectable | ItemSendsGeometryChanges);
    setToolTip(getName());
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);
    connect(this, &Database::setMainWinTitle, mainwin, &MainWin::setWindowTitle);
    connect(this, &Database::expandDatabase, mainwin, &MainWin::showSchemas);
    connect(this, &Database::collapseDatabase, mainwin, &MainWin::hideSchemas);
    connect(this, &Database::expandAll, mainwin, &MainWin::explodeAndShowSchemas);
    connect(this, &Database::expandAllVertically, mainwin, &MainWin::explodeAndShowSchemasVertically);
}

void Database::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    //if(!getDatabaseStatus()) {
        emit summonPropertyDialog(this);
        //return;
    //}
    /*
    if(getDatabaseStatus() && getDatabaseCollapsed()) {
        setDatabaseCollapsed(false);
        emit expandDatabase(this);
    }
    else if(getDatabaseStatus() && !getDatabaseCollapsed()) {
        setDatabaseCollapsed(true);
        emit collapseDatabase(this);
    }*/
    //update();
}

void Database::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    if(getDatabaseStatus()) {
        if(getDatabaseCollapsed())
            menu.addAction(tr("Explode"));
        else
            menu.addAction(tr("Collapse"));
        //menu.addAction("Explode all");
        //menu.addAction("Explode all vertically");
        menu.addSeparator();
        menu.addAction(tr("Refresh"));
        menu.addSeparator();
    }
    menu.addAction(QIcon(":/icons/properties.svg"), tr("Properties"));
    QAction *process = menu.addAction(tr("Status"));
    if(getDatabaseStatus()) {
        process->setEnabled(true);
    }
    else {
        process->setEnabled(false);
    }

    QAction *a = menu.exec(event->screenPos());
    if(a && QString::compare(a->text(), tr("Explode")) == 0) {
        emit expandDatabase(this);
        setDatabaseCollapsed(false);
        update();
    }
    else if(a && QString::compare(a->text(), tr("Collapse")) == 0) {
        emit collapseDatabase(this);
        setDatabaseCollapsed(true);
        update();
    }
    else if(a && QString::compare(a->text(), tr("Explode all"))==0) {
        emit expandAll(this);
        setDatabaseCollapsed(false);
        update();
    }
    else if(a && QString::compare(a->text(), tr("Explode all vertically"))==0) {
        emit expandAllVertically(this);
        setDatabaseCollapsed(false);
        update();
    }
    else if(a && QString::compare(a->text(), tr("Properties"))==0) {
        emit summonPropertyDialog(this);
    }
    else if(a && QString::compare(a->text(), tr("Status"))==0) {
        emit summonStatusDialog(this);
    }
    else if(a && QString::compare(a->text(), tr("Refresh"))==0) {
        populateDatabase();
        if(mainwin->getSearchBox()->isVisible() && !mainwin->getSearchBox()->text().isEmpty()) {
            QString search_term = mainwin->getSearchBox()->text();
            if(mainwin->displayMode() == MainWin::Tables) {
                foreach(Schema *schema, getSchemaList()) {
                    foreach(Table *table, schema->getTableList()) {
                        if(table->getName().contains(search_term))
                            table->setSearched(true);
                        else
                            table->setSearched(false);
                        update();
                    }
                }
            }
            else if(mainwin->displayMode() == MainWin::Views) {
                foreach(Schema *schema, getSchemaList()) {
                    foreach(View *view, schema->getViewList()) {
                        if(view->getName().contains(search_term))
                            view->setSearched(true);
                        else
                            view->setSearched(false);
                        update();
                    }
                }
            }
            else if(mainwin->displayMode() == MainWin::Functions) {
                foreach(Schema *schema, getSchemaList()) {
                    foreach(Function *function, schema->getFunctionList()) {
                        if(function->getName().contains(search_term))
                            function->setSearched(true);
                        else
                            function->setSearched(false);
                        update();
                    }
                }
            }
        }
    }
}

void Database::delDatabase(Database *db)
{
    delete db;
}

bool Database::populateDatabase()
{
    mainwin->clearSchemas();
    table_names_list.clear();
    view_names_list.clear();
    function_names_list.clear();
    keywords_list.clear();

    QList<Schema*> schema_list;
    {
        QSqlQuery *query = new QSqlQuery(database_connection);
        query->exec("SELECT DISTINCT nspname FROM pg_namespace WHERE \
                   nspname NOT LIKE 'pg_%' AND nspname<>'information_schema' \
                   ORDER BY nspname;");
        if(query->lastError().isValid())
        {
            QMessageBox *error_message = new QMessageBox(QMessageBox::Critical,
                                        tr("Database error"),
                                        tr("Unable to retrieve database schemas.\n"
                                           "Check your database connection or permissions.\n"),
                                        QMessageBox::Cancel,0,Qt::Dialog);
            error_message->setWindowModality(Qt::NonModal);
            error_message->show();
            return false;
        }
        number_of_schemas = query->size();
        while (query->next())
        {
            QString schema_name = query->value(0).toString();
            Schema *schema = new Schema(mainwin, this, schema_name, schema_list.size(), number_of_schemas);
            schema->resetPos();

            /*SchemaLink *schema_link = new SchemaLink(this, schema);
            if(mainwin->isColumnView())
                schema_link->hide();
            else
                schema_link->show();
            schema_link->show();*/

            QObject::connect(schema, &Schema::expandSchemaTables, mainwin, &MainWin::showTables);
            QObject::connect(schema, &Schema::collapseSchemaTables, mainwin, &MainWin::hideTables);
            QObject::connect(schema, &Schema::collapseOtherSchemas, mainwin, &MainWin::hideOtherTables);
            QObject::connect(schema, &Schema::expandSchemaViews, mainwin, &MainWin::showViews);
            QObject::connect(schema, &Schema::collapseSchemaViews, mainwin, &MainWin::hideViews);
            QObject::connect(schema, &Schema::expandSchemaFunctions, mainwin, &MainWin::showFunctions);
            QObject::connect(schema, &Schema::collapseSchemaFunctions, mainwin, &MainWin::hideFunctions);
            QObject::connect(schema, &Schema::newTable, mainwin, &MainWin::newTable);
            //QObject::connect(schema, &Schema::newView, mainwin, &MainWin::newView);
            QObject::connect(schema, &Schema::newFunction, mainwin, &MainWin::newFunction);
            connect(this, &Database::collapseSchemaTables, mainwin, &MainWin::hideTables);
            connect(this, &Database::collapseSchemaFunctions, mainwin, &MainWin::hideFunctions);
            if(!mainwin->isColumnView()) {
                emit collapseSchemaTables(schema);
            }
            emit collapseSchemaViews(schema);
            emit collapseSchemaFunctions(schema);
            schema->update();
            schema_list.append(schema);
        }

        query->exec("SELECT oid, * FROM pg_type;");
        if(query->lastError().isValid()) {
            QMessageBox *error_message = new QMessageBox(QMessageBox::Critical,
                                        tr("Database error"),
                                        tr("Unable to retrieve data types.\n"
                                           "Check your database connection or permissions.\n"),
                                        QMessageBox::Cancel,0,Qt::Dialog);
            error_message->setWindowModality(Qt::NonModal);
            error_message->show();
            return false;
        }
        while (query->next()) {
            types_hash.insert(query->value(0).toInt(),query->value(1).toString());
        }

        query->exec("select word, upper(word) from pg_get_keywords()");
        if(query->lastError().isValid()) {
            QMessageBox *error_message = new QMessageBox(QMessageBox::Critical,
                                        tr("Database error"),
                                        tr("Unable to retrieve data types.\n"
                                           "Check your database connection or permissions.\n"),
                                        QMessageBox::Cancel,0,Qt::Dialog);
            error_message->setWindowModality(Qt::NonModal);
            error_message->show();
            return false;
        }
        while (query->next()) {
            keywords_list.append(query->value(0).toString());
            keywords_list.append(query->value(1).toString());
        }

        query->exec("show all");
        if(query->lastError().isValid()) {
            QMessageBox *error_message = new QMessageBox(QMessageBox::Critical,
                                        tr("Database error"),
                                        tr("Unable to retrieve database settings.\n"
                                           "Check your database connection or permissions.\n"),
                                        QMessageBox::Cancel,0,Qt::Dialog);
            error_message->setWindowModality(Qt::NonModal);
            error_message->show();
            return false;
        }
        while (query->next()) {
            settings_list.insert(query->value(0).toString(),query->value(1).toString());
        }
    }

    setSchemaList(schema_list);
    setDatabaseStatus(true);
    if(mainwin->displayMode() == MainWin::Tables)
        mainwin->showAllTables();
    else if(mainwin->displayMode() == MainWin::Views)
        mainwin->showAllViews();
    else if(mainwin->displayMode() == MainWin::Functions)
        mainwin->showAllFunctions();
    return true;
}

/*void Database::refresh()
{

}*/

bool Database::setConnectionProperties(const QString srv,
                            const qint32 port,
                            const QString dbname,
                            const QString user,
                            const QString pass)
{
    QSqlDatabase::removeDatabase(QString("base").append(QString::number(database_id)));
    QStringList drivers(database_connection.drivers());
    if(!drivers.contains("QPSQL")) {
        QMessageBox::critical(mainwin, tr("Database error"),
            tr("Unable to establish a database connection.\n"
               "No PostgreSQL support.\n"), QMessageBox::Cancel);
        return false;
    }

    setHost(srv);
    setPort(QString::number(port));
    setName(dbname);
    setUser(user);
    setPassword(pass);

    setParamStatus(true);

    database_connection = QSqlDatabase::addDatabase("QPSQL", QString("base").append(QString::number(database_id)));

    database_connection.setHostName(srv);
    database_connection.setPort(port);
    database_connection.setDatabaseName(dbname);
    database_connection.setUserName(user);
    database_connection.setPassword(pass);
    setName(database_connection.databaseName());
    if (!database_connection.open()) {
        QMessageBox::critical(mainwin, tr("Database error"),
            tr("Couldn't connect to database.\n"
               "Check connection parameters.\n"), QMessageBox::Cancel);
        MainWin::session_unsaved = false;
        return false;
    }

    if (!populateDatabase())
        return false;
    setDatabaseCollapsed(false);
    update();
    return true;
}

bool Database::advance()
{
    if (newPos == pos())
        return false;
    setPos(newPos);
    return true;
}

void Database::arrangeHorizontally()
{
    populateDatabase();
    QList<Schema*> schema_list = getSchemaList();
    foreach(Schema *schema, schema_list)
    {
        schema->horizontalPosition();
    }
    //scene()->setSceneRect(QRectF());
}

/*void Database::newSchema(QString new_schema_name)
{

}*/

QVariant Database::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemPositionHasChanged:
        foreach (SchemaLink *edge, edgeList)
            edge->adjust();
        break;
    default:
        break;
    }

    return QGraphicsItem::itemChange(change, value);
}

void Database::addEdge(SchemaLink *edge)
{
    edgeList << edge;
    edge->adjust();
}
