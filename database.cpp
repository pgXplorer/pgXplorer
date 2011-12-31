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
    setDatabaseCollapsed(true);
    setFlags(QGraphicsItem::ItemIsSelectable | ItemSendsGeometryChanges);
    setToolTip(getName());
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);
    connect(this, SIGNAL(setMainWinTitle(QString)), mainwin, SLOT(setWindowTitle(QString)));
    connect(this, SIGNAL(expandDatabase(Database*)), mainwin, SLOT(showSchemas()));
    connect(this, SIGNAL(collapseDatabase(Database*)), mainwin, SLOT(hideSchemas()));
    connect(this, SIGNAL(expandAll(Database*)), mainwin, SLOT(explodeAndShowSchemas()));
    connect(this, SIGNAL(expandAllVertically(Database*)), mainwin, SLOT(explodeAndShowSchemasVertically()));
}

void Database::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if(!getDatabaseStatus()) {
        showPropertyDialog();
        return;
    }/*
    if(getDatabaseStatus() && getDatabaseCollapsed()) {
        setDatabaseCollapsed(false);
        emit expandDatabase(this);
    }
    else if(getDatabaseStatus() && !getDatabaseCollapsed()) {
        setDatabaseCollapsed(true);
        emit collapseDatabase(this);
    }*/
    update();
}

void Database::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    if(getDatabaseStatus())
    {
        if(getDatabaseCollapsed())
            menu.addAction("Explode");
        else
            menu.addAction("Collapse");
        //menu.addAction("Explode all");
        //menu.addAction("Explode all vertically");
        menu.addSeparator();
    }
    menu.addAction(QIcon(qApp->applicationDirPath().append("/icons/properties.png")), QApplication::translate("Database", "Properties", 0, QApplication::UnicodeUTF8));
    QAction *a = menu.exec(event->screenPos());
    if(a && QString::compare(a->text(),"Explode")==0) {
        emit expandDatabase(this);
        setDatabaseCollapsed(false);
        update();
    }
    else if(a && QString::compare(a->text(),"Collapse")==0) {
        emit collapseDatabase(this);
        setDatabaseCollapsed(true);
        update();
    }
    else if(a && QString::compare(a->text(),"Explode all")==0) {
        emit expandAll(this);
        setDatabaseCollapsed(false);
        update();
    }
    else if(a && QString::compare(a->text(),"Explode all vertically")==0) {
        emit expandAllVertically(this);
        setDatabaseCollapsed(false);
        update();
    }
    else if(a && QString::compare(a->text(),"Properties")==0) {
        showPropertyDialog();
    }
}

void Database::showPropertyDialog()
{
    if(!getDatabaseStatus()) {
        setHost("127.0.0.1");
        setPort("5432");
        setUser("postgres");
        setPassword("");
    }
    ConnectionProperties *connection_properties = new ConnectionProperties(this, mainwin);
    connection_properties->exec();
}

void Database::delDatabase(Database *db)
{
    delete db;
}

bool Database::populateDatabase()
{
    mainwin->table_completer_list.clear();
    mainwin->view_completer_list.clear();
    mainwin->function_completer_list.clear();
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
                                        "Check your database connection or permissions.\n"), QMessageBox::Cancel,0,Qt::Dialog);
            error_message->setWindowModality(Qt::NonModal);
            error_message->show();
            return false;
        }
        number_of_schemas = query->size();
        while (query->next())
        {
            QString schema_name = query->value(0).toString();
            Schema *schema = new Schema(mainwin, this, schema_name, schema_list.size(), number_of_schemas);
            /*SchemaLink *schema_link = new SchemaLink(this, schema);
            if(mainwin->isColumnView())
                schema_link->hide();
            else
                schema_link->show();
            schema_link->show();*/

            QObject::connect(schema, SIGNAL(expandSchemaTables(Schema*)), mainwin, SLOT(showTables(Schema*)));
            QObject::connect(schema, SIGNAL(collapseSchemaTables(Schema*)), mainwin, SLOT(hideTables(Schema*)));
            QObject::connect(schema, SIGNAL(collapseOtherSchemas(Schema*)), mainwin, SLOT(hideOtherTables(Schema*)));
            QObject::connect(schema, SIGNAL(expandSchemaViews(Schema*)), mainwin, SLOT(showViews(Schema*)));
            QObject::connect(schema, SIGNAL(collapseSchemaViews(Schema*)), mainwin, SLOT(hideViews(Schema*)));
            QObject::connect(schema, SIGNAL(expandSchemaFunctions(Schema*)), mainwin, SLOT(showFunctions(Schema*)));
            QObject::connect(schema, SIGNAL(collapseSchemaFunctions(Schema*)), mainwin, SLOT(hideFunctions(Schema*)));
            connect(this, SIGNAL(collapseSchemaTables(Schema*)), mainwin, SLOT(hideTables(Schema*)));
            connect(this, SIGNAL(collapseSchemaFunctions(Schema*)), mainwin, SLOT(hideFunctions(Schema*)));
            if(!mainwin->isColumnView()) {
                emit collapseSchemaTables(schema);
            }
            emit collapseSchemaViews(schema);
            emit collapseSchemaFunctions(schema);
            schema->update();
            schema_list.append(schema);
        }

        query->exec("SELECT oid, * FROM pg_type;");
        if(query->lastError().isValid())
        {
            QMessageBox *error_message = new QMessageBox(QMessageBox::Critical,
                                        tr("Database error"),
                                        tr("Unable to retrieve database schemas.\n"
                                        "Check your database connection or permissions.\n"), QMessageBox::Cancel,0,Qt::Dialog);
            error_message->setWindowModality(Qt::NonModal);
            error_message->show();
            return false;
        }
        while (query->next())
        {
            types_hash.insert(query->value(0).toInt(),query->value(1).toString());
        }
    }
    setSchemaList(schema_list);
    setDatabaseStatus(true);
    return true;
}

bool Database::setConnectionProperties(const QString srv,
                            const qint32 port,
                            const QString dbname,
                            const QString user,
                            const QString pass)
{
    QSqlDatabase::removeDatabase(QString("base").append(QString::number(database_id)));
    QStringList drivers(database_connection.drivers());
    if(!drivers.contains("QPSQL")) {
        QMessageBox::critical(0, tr("Database error"),
            tr("Unable to establish a database connection.\n"
                     "No PostgreSQL support.\n"), QMessageBox::Cancel);
        return false;
    }

    setHost(srv);
    setPort(QString::number(port));
    setName(dbname);
    setUser(user);
    setPassword(pass);

    database_connection = QSqlDatabase::addDatabase("QPSQL", QString("base").append(QString::number(database_id)));
    database_connection.setHostName(srv);
    database_connection.setPort(port);
    database_connection.setDatabaseName(dbname);
    database_connection.setUserName(user);
    database_connection.setPassword(pass);
    setName(database_connection.databaseName());
    if (!database_connection.open()) {
        QMessageBox::critical(0, tr("Database error"),
            tr("Couldn't connect to database.\n"
                     "Check connection parameters.\n"), QMessageBox::Cancel);
        MainWin::document_unsaved = false;
        return false;
    }
    if (!populateDatabase()) {
        return false;
    }
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
