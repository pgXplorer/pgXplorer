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

#include "schema.h"
#include "database.h"

Schema::Schema(MainWin *mainwin, Database *database, QString schema_name, int schema_index, uint number_of_schemas)
{
    this->mainwin = mainwin;
    this->schema_index = schema_index;
    this->number_of_schemas = number_of_schemas;
    qreal dtheta = -2*M_PI*schema_index/number_of_schemas - M_PI_2;
    setParent(database);
    setParentItem(database);
    /*
    if(mainwin->isColumnView())
        setPos((a_radius/2)*(schema_index) + a_radius/2/2 -(parent_database->getSchemaCount()*a_radius/2/2), b_radius/2);
    else
        setPos(a_radius*sin(dtheta), b_radius*cos(dtheta));
    */
    setName(schema_name);
    setStatus(false);
    setSchemaCollapsed(true);

    setFlag(ItemIsSelectable);
    //setFlag(ItemSendsGeometryChanges);

    setToolTip(this->getName());
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);
    /*if(mainwin->isColumnView()) {
        populateSchemaTablesColumnwise();
        populateSchemaFunctionsColumnwise();
    }
    else {
        populateSchemaTables();
        populateSchemaFunctions();
    }*/
    populateSchemaTables();
    populateSchemaViews();
    populateSchemaFunctions();
}

void Schema::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
{
    if(getSchemaCollapsed()) {
        setSchemaCollapsed(false);
        if(mainwin->isColumnView()) {
            if(mainwin->isTableView())
                emit expandSchemaTables(this);
            else if(mainwin->isViewView())
                emit expandSchemaViews(this);
            else if(mainwin->isFunctionView())
                emit expandSchemaFunctions(this);
        }
        else {
            emit expandSchemaTables(this);
        }
    }
    else {
        setSchemaCollapsed(true);
        if(mainwin->isColumnView()) {
            if(mainwin->isTableView())
                emit collapseSchemaTables(this);
            else if(mainwin->isViewView())
                emit collapseSchemaViews(this);
            else if(mainwin->isFunctionView())
                emit collapseSchemaFunctions(this);
        }
        else {
            emit expandSchemaTables(this);
        }
    }
}

void Schema::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    MainWin::DisplayMode display_mode = mainwin->displayMode();

    QMenu menu;
    menu.addAction(tr("Refresh"));
    if(display_mode == MainWin::Views){
        //menu.addAction(tr("New view"));
    }
    else if(display_mode == MainWin::Functions) {
        //menu.addAction(tr("New function"));
    }
    else if (display_mode == MainWin::Tables) {
        menu.addAction(tr("New table"));
    }

    QAction *a = menu.exec(event->screenPos());

    if(a && QString::compare(a->text(),tr("Refresh")) == 0) {
        if(display_mode == MainWin::Views)
            resetViewsVertically2();
        else if(display_mode == MainWin::Functions)
            resetFunctionsVertically2();
        else if(display_mode == MainWin::Tables)
            resetTablesVertically2();
    }
    else if(a && QString::compare(a->text(),tr("New view")) == 0) {
        emit newView(this);
    }
    else if(a && QString::compare(a->text(),tr("New function")) == 0) {
        emit newFunction(this);
    }
    else if(a && QString::compare(a->text(),tr("New table")) == 0) {
        emit newTable(this);
    }
}

bool Schema::advance()
{
    if (newPos == pos())
        return false;
    setPos(newPos);
    return true;
}

void Schema::populateSchemaTables()
{
    foreach (Table *table, getTableList())
        delete table;
    this->table_list.clear();
    this->setTableCount(0);

    QList<Table*> table_list;
    QSqlQuery table_query(parent_database->getDatabaseConnection());
    QString table_query_string = "SELECT 0, tablename FROM pg_tables WHERE schemaname='"+this->getName()+"' ORDER BY 1,2";
    table_query.exec(table_query_string);
    setTableCount(table_query.size());
    if(table_query.lastError().isValid()) {
        QMessageBox *error_message = new QMessageBox(QMessageBox::Critical,
                                    tr("Database error"),
                                    tr("Unable to retrieve schema tables.\n"
                                       "Check your database connection or permissions.\n"),
                                    QMessageBox::Cancel,0,Qt::Dialog);
        error_message->setWindowModality(Qt::NonModal);
        error_message->show();
        return;
    }
    while (table_query.next()) {
        QString table_name = table_query.value(1).toString();
        Table *table;
        if(table_query.value(0).toInt() == 0) {
            table = new Table(parent_database, this, table_name, table_list.size(), QColor(100,50,50));
            table->setView(false);
        }
        else
            return;

        table->setSearched(true);
        if(mainwin->isColumnView())
            table->verticalPosition2();
        else
            table->defaultPosition();

        QObject::connect(mainwin->getSearchBox(), SIGNAL(textChanged(QString)), table, SLOT(getSearchTerm(QString)));
        QObject::connect(mainwin, SIGNAL(showColumnView()), table, SLOT(verticalPosition2()));
        QObject::connect(table, SIGNAL(expandTable(Database *, Schema *, Table*)), mainwin, SLOT(showTableView(Database *, Schema *, Table*)));
        QObject::connect(table, SIGNAL(designTable(Database *, Schema *, Table*)), mainwin, SLOT(showDesignView(Database *, Schema *, Table*)));
        //QObject::connect(table, SIGNAL(rename(Database *, Schema *, Table*)), mainwin, SLOT(renameTable(Database *, Schema *, Table*)));
        QObject::connect(table, SIGNAL(clearTable(Database *, Schema *, Table*)), mainwin, SLOT(clearTableView(Database *, Schema *, Table*)));
        QObject::connect(table, SIGNAL(dropTable(Database *, Schema *, Table*)), mainwin, SLOT(dropTable(Database *, Schema *, Table*)));

        table_list.append(table);
        if(!mainwin->table_completer_list.contains(table_name))
            mainwin->table_completer_list.append(table_name);
    }
    setTableList(table_list);
}

void Schema::populateSchemaViews()
{
    foreach (View *view, getViewList())
        delete view;
    this->view_list.clear();
    this->setViewCount(0);

    QList<View*> view_list;
    QSqlQuery *view_query = new QSqlQuery(parent_database->getDatabaseConnection());
    QString view_query_string = "SELECT 1, viewname FROM pg_views WHERE schemaname='"+this->getName()+"' ORDER BY 1,2";
    view_query->exec(view_query_string);
    setViewCount(view_query->size());
    if(view_query->lastError().isValid()) {
        QMessageBox *error_message = new QMessageBox(QMessageBox::Critical,
                                    tr("Database error"),
                                    tr("Unable to retrieve schema views.\n"
                                    "Check your database connection or permissions.\n"), QMessageBox::Cancel,0,Qt::Dialog);
        error_message->setWindowModality(Qt::NonModal);
        error_message->show();
        return;
    }

    while (view_query->next()) {
        QString view_name = view_query->value(1).toString();
        View *view;
        if(view_query->value(0).toInt() == 1)
            view = new View(parent_database, this, view_name, view_list.size(), QColor(100,50,50));
        else
            return;

        view->setSearched(true);
        if(mainwin->isColumnView())
            view->verticalPosition2();
        else
            view->defaultPosition();

        QObject::connect(mainwin->getSearchBox(), SIGNAL(textChanged(QString)), view, SLOT(getSearchTerm(QString)));
        QObject::connect(mainwin, SIGNAL(showColumnView()), view, SLOT(verticalPosition2()));
        QObject::connect(view, SIGNAL(expandView(Database *, Schema *, View*)), mainwin, SLOT(showViewView(Database *, Schema *, View*)));
        QObject::connect(view, SIGNAL(expandViewDefinition(Schema *, View*)), mainwin, SLOT(showViewEditor(Schema*, View*)));
        QObject::connect(view, SIGNAL(dropView(Database *, Schema *, View*)), mainwin, SLOT(dropView(Database *, Schema *, View*)));

        view_list.append(view);
        if(!mainwin->view_completer_list.contains(view_name))
            mainwin->view_completer_list.append(view_name);
    }
    setViewList(view_list);
}

void Schema::populateSchemaFunctions()
{
    foreach (Function *func, getFunctionList())
        delete func;
    this->function_list.clear();
    setFunctionCount(0);

    QList<Function*> function_list;
    QSqlQuery *function_query = new QSqlQuery(parent_database->getDatabaseConnection());
    QString function_query_string = "SELECT proname,proargnames,proargtypes FROM pg_catalog.pg_namespace n JOIN pg_catalog.pg_proc p ON pronamespace = n.oid WHERE nspname='"+this->getName()+"' ORDER BY 1,2,3";
    function_query->exec(function_query_string);

    setFunctionCount(function_query->size());
    if(function_query->lastError().isValid()) {
        QMessageBox *error_message = new QMessageBox(QMessageBox::Critical,
                                    tr("Database error"),
                                    tr("Unable to retrieve schema tables.\n"
                                       "Check your database connection or permissions.\n"),
                                    QMessageBox::Cancel,0,Qt::Dialog);
        error_message->setWindowModality(Qt::NonModal);
        error_message->show();
        return;
    }

    while (function_query->next()) {
        QString function_name = function_query->value(0).toString();
        QString function_args = function_query->value(1).toString();
        QString function_arg_types = function_query->value(2).toString();

        Function *function = new Function(parent_database, this, function_name, function_args, function_arg_types, function_list.size(), QColor(100,100,50));
        function->setSearched(true);

        if(mainwin->isColumnView())
            function->verticalPosition2();
        else
            function->defaultPosition();

        QObject::connect(mainwin->getSearchBox(), SIGNAL(textChanged(QString)), function, SLOT(getSearchTerm(QString)));
        //QObject::connect(mainwin, SIGNAL(), function, SLOT(getSearchTerm(QString)));
        QObject::connect(mainwin, SIGNAL(showColumnView()), function, SLOT(verticalPosition2()));
        QObject::connect(function, SIGNAL(expandFunction(Schema*, Function*)), mainwin, SLOT(showFunctionEditor(Schema*, Function*)));
        //QObject::connect(function, SIGNAL(runFunction(Database *, Schema *, Function*)), mainwin, SLOT(runFunction(Database *, Schema *, Function*)));
        QObject::connect(function, SIGNAL(dropFunction(Database *, Schema *, Function*)), mainwin, SLOT(dropFunction(Database *, Schema *, Function*)));

        function_list.append(function);
        if(!mainwin->function_completer_list.contains(function_name))
            mainwin->function_completer_list.append(function_name);
    }
    setFunctionList(function_list);
}

void Schema::resetPos()
{
    setPos((A_RADIUS/2)*(schema_index) + A_RADIUS/2/2 -(number_of_schemas*A_RADIUS/2/2), B_RADIUS/2);
}

void Schema::resetTables()
{
    populateSchemaTables();
    QList<Table*> table_list = getTableList();
    foreach(Table *table, table_list) {
        if(mainwin->getSearchBox()->isVisible())
            table->getSearchTerm(mainwin->getSearchBox()->text());
        table->defaultPosition();
    }
    scene()->setSceneRect(QRectF());
}

void Schema::resetTablesVertically()
{
    populateSchemaTables();
    QList<Table*> table_list = getTableList();
    foreach(Table *table, table_list) {
        if(mainwin->getSearchBox()->isVisible())
            table->getSearchTerm(mainwin->getSearchBox()->text());
        table->verticalPosition();
    }
    scene()->setSceneRect(QRectF());
}

void Schema::resetTablesVertically2()
{
    populateSchemaTables();
    QList<Table*> table_list = getTableList();
    foreach(Table *table, table_list) {
        if(mainwin->getSearchBox()->isVisible())
            table->getSearchTerm(mainwin->getSearchBox()->text());
        table->verticalPosition2();
    }
    scene()->setSceneRect(QRectF());
}

void Schema::resetViews()
{
    populateSchemaViews();
    QList<View*> view_list = getViewList();
    foreach(View *view, view_list) {
        if(mainwin->getSearchBox()->isVisible())
            view->getSearchTerm(mainwin->getSearchBox()->text());
        view->defaultPosition();
    }
    scene()->setSceneRect(QRectF());
}

void Schema::resetViewsVertically2()
{
    populateSchemaViews();
    QList<View*> view_list = getViewList();
    foreach(View *view, view_list) {
        if(mainwin->getSearchBox()->isVisible())
            view->getSearchTerm(mainwin->getSearchBox()->text());
        view->verticalPosition2();
    }
    scene()->setSceneRect(QRectF());
}

void Schema::resetFunctions()
{
    populateSchemaFunctions();
    QList<Function*> function_list = getFunctionList();
    foreach(Function *function, function_list) {
        if(mainwin->getSearchBox()->isVisible())
            function->getSearchTerm(mainwin->getSearchBox()->text());
        function->defaultPosition();
    }
    scene()->setSceneRect(QRectF());
}

void Schema::resetFunctionsVertically2()
{
    populateSchemaFunctions();
    QList<Function*> function_list = getFunctionList();
    foreach(Function *function, function_list) {
        if(mainwin->getSearchBox()->isVisible())
            function->getSearchTerm(mainwin->getSearchBox()->text());
        function->verticalPosition2();
    }
    scene()->setSceneRect(QRectF());
}

void Schema::horizontalPosition()
{
    float xs = parent_database->x();
    float ys = 0;
    float i;
    int schema_count = parent_database->getSchemaCount();
    if(schema_count%2 == 0) {
        if (xs < 0)
            i = -schema_index+(schema_count/2)-0.5;
        else
            i = schema_index-(schema_count/2)+0.5;
    }
    else {
        if (xs < 0)
            i = -schema_index+(schema_count/2);
        else
            i = schema_index-(schema_count/2);
    }
    int radius = 8*schema_count;
    if(radius < 100) radius = 100;
    qreal dtheta = 2*M_PI*i/schema_count;
    if(xs < 0) {
        setPos((-radius*cos(atan(ys/xs))+radius*(dtheta)*sin(atan(ys/xs))),
                     (-radius*sin(atan(ys/xs))-radius*(dtheta)*cos(atan(ys/xs))));
    }
    else if(xs > 0) {
        setPos(radius*cos(atan(ys/xs))-radius*(dtheta)*sin(atan(ys/xs)),
                     radius*sin(atan(ys/xs))+radius*(dtheta)*cos(atan(ys/xs)));
    }
}

/*
QVariant Schema::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemPositionHasChanged:
        schema_link->adjust();
        break;
    default:
        break;
    };

    return QGraphicsItem::itemChange(change, value);
}*/

void Schema::addEdge(SchemaLink *a_schema_link)
{
    schema_link = a_schema_link;
    a_schema_link->adjust();
}

/*
void Schema::addEdge(TableLink *a_table_link)
{
    table_link_list << a_table_link;
    a_table_link->adjust();
}

void Schema::addEdge(FunctionLink *a_function_link)
{
    function_link_list << a_function_link;
    a_function_link->adjust();
}

SchemaLink *Schema::dblink() const
{
    return schema_link;
}

QList<TableLink *> Schema::tablelinks() const
{
    return table_link_list;
}
*/
