/*
  LICENSE AND COPYRIGHT INFORMATION - Please read carefully.

  Copyright (c) 2011-2012, davyjones <dj@pgxplorer.com>

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

#include "mainwin.h"
#include "database.h"
#include "table.h"
#include "schema.h"

Table::Table(Database *database, Schema *schema, QString table_name, int table_index, QColor color)
{
    this->database = database;
    oid = false;
    setParent(schema);
    setParentItem(schema);
    this->table_index = table_index;
    setName(table_name);
    ascii_length = table_name.toLatin1().length();
    utf8_length = table_name.toUtf8().length();
    setStatus(false);
    setCollapsed(true);
    createBrush();
    setFlags(QGraphicsItem::ItemIsSelectable);// | QGraphicsItem::ItemIsMovable);
    //setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-10);
    setAcceptHoverEvents(true);
}

QString Table::getFullName()
{
    return "\"" + parent_schema->getName() + "\".\"" + table_name + "\"";
}

void Table::createBrush()
{
    QRadialGradient pink_gradient(50,50,100,50,50);
    pink_gradient.setColorAt(1, QColor::fromRgbF(0.8, 0.4, 0.4, 0.4));
    pink_gradient.setColorAt(0, QColor::fromRgbF(0, 0, 0, 0));
    pink_brush = QBrush(pink_gradient);
}

void Table::defaultPosition()
{
    float xs = parent_schema->x();
    float ys = parent_schema->y();
    float i;
    int table_count = parent_schema->getTableCount();
    if(table_count%2 == 0) {
        if (xs < 0)
            i = -table_index+(table_count/2)-0.5;
        else
            i = table_index-(table_count/2)+0.5;
    }
    else {
        if (xs < 0)
            i = -table_index+(table_count/2);
        else
            i = table_index-(table_count/2);
    }
    int radius = 8*table_count;
    if(radius < 100) radius = 100;
    qreal dtheta = 2*M_PI*i/table_count;
    if(xs < 0)
    {
        setPos((-radius*cos(atan(ys/xs))+radius*(dtheta)*sin(atan(ys/xs))),
                     (-radius*sin(atan(ys/xs))-radius*(dtheta)*cos(atan(ys/xs))));
    }
    else if(xs > 0)
    {
        setPos(radius*cos(atan(ys/xs))-radius*(dtheta)*sin(atan(ys/xs)),
                     radius*sin(atan(ys/xs))+radius*(dtheta)*cos(atan(ys/xs)));
    }
}

void Table::setColumnData()
{
    QSqlQuery column_query(database->getDatabaseConnection());
    QString column_query_string = "SELECT a.attnum, a.attname, pg_catalog.format_type(a.atttypid, a.atttypmod), a.atttypmod-4, attnotnull, (SELECT substring(pg_catalog.pg_get_expr(d.adbin, d.adrelid) for 128) FROM pg_catalog.pg_attrdef d WHERE d.adrelid = a.attrelid AND d.adnum = a.attnum AND a.atthasdef) FROM pg_catalog.pg_attribute a WHERE a.attrelid in (SELECT c.oid FROM pg_catalog.pg_class c LEFT JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace WHERE n.nspname='" + parent_schema->getName() + "' and c.relname='" + table_name + "') AND (a.attnum = -2 OR a.attnum > 0) AND NOT a.attisdropped ORDER BY a.attnum";
    column_query.exec(column_query_string);
    if(column_query.lastError().isValid()) {
        QMessageBox *error_message = new QMessageBox(QMessageBox::Critical,
                                    tr("Database error"),
                                    tr("Unable to retrieve table information.\n"
                                    "Check your database connection or permissions.\n"), QMessageBox::Cancel,0,Qt::Dialog);
        error_message->setWindowModality(Qt::NonModal);
        error_message->show();
        return;
    }
    //Clear the column list just before populating it.
    column_list.clear();
    column_types.clear();
    column_lengths.clear();
    column_nulls.clear();
    column_defaults.clear();
    while (column_query.next()) {
        if(column_query.value(0).toInt() < 0)
             oid = true;
        else {
            column_list.append("\"" + column_query.value(1).toString() + "\"");
            column_types.append(column_query.value(2).toString());
            column_lengths.append(column_query.value(3).toString());
            column_nulls.append(column_query.value(4).toString());
            column_defaults.append(column_query.value(5).toString());
        }
    }
}

void Table::copyPrimaryKey()
{
    QSqlQuery column_query(database->getDatabaseConnection());
    QString column_query_string = "select conname, unnest(conkey)::int-1 from pg_constraint p left join pg_namespace n on p.connamespace=n.oid left join pg_class c on c.oid=conrelid where n.nspname='" + parent_schema->getName() + "' and c.relname='" + table_name + "' and contype in ('p','u') and conrelid > 0";
    //QString column_query_string = "select pg_get_constraintdef(where n.nspname='" + parent_schema->getName() + "' and c.relname='" + table_name + "', true);";
    column_query.exec(column_query_string);
    if(column_query.lastError().isValid()) {
        QMessageBox *error_message = new QMessageBox(QMessageBox::Critical,
                                    tr("Database error"),
                                    tr("Unable to retrieve primary key information.\n"
                                    "Check your database connection or permissions.\n"), QMessageBox::Cancel,0,Qt::Dialog);
        error_message->setWindowModality(Qt::NonModal);
        error_message->show();
        return;
    }

    primary_key.clear();
    primary_key_has_oid = false;

    while (column_query.next()) {
        primary_key_name = column_query.value(0).toString();
        if(column_query.value(1).toInt() < 0)
            primary_key_has_oid = true;
        else
            primary_key.append(column_list.at(column_query.value(1).toInt()));
    }
}

void Table::copyConstraints()
{
    QSqlQuery constraint_query(database->getDatabaseConnection());
    QString query_string = "SELECT r.conname, pg_catalog.pg_get_constraintdef(r.oid, true) FROM pg_catalog.pg_constraint r WHERE r.conrelid IN (SELECT c.oid FROM pg_catalog.pg_class c LEFT JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace WHERE n.nspname='" + parent_schema->getName() + "' and c.relname='" + table_name + "') AND r.contype in ('c') ORDER BY r.oid";
    constraint_query.exec(query_string);
    if(constraint_query.lastError().isValid()) {
        QMessageBox *error_message = new QMessageBox(QMessageBox::Critical,
                                    tr("Database error"),
                                    tr("Unable to retrieve constraint information.\n"
                                    "Check your database connection or permissions.\n"), QMessageBox::Cancel,0,Qt::Dialog);
        error_message->setWindowModality(Qt::NonModal);
        error_message->show();
        return;
    }

    table_constraint_names.clear();
    table_constraints.clear();

    while (constraint_query.next()) {
        QString constraint("    CONSTRAINT \"");
        constraint.append(constraint_query.value(0).toString() + "\" " + constraint_query.value(1).toString());
        table_constraints.append(constraint);
    }
}

void Table::verticalPosition()
{
    float xs = parent_schema->x();
    float ys = 0;
    float i;
    int table_count = parent_schema->getTableCount();
    if(table_count%2 == 0) {
        if (xs < 0)
            i = -table_index+(table_count/2)-0.5;
        else
            i = table_index-(table_count/2)+0.5;
    }
    else {
        if (xs < 0)
            i = -table_index+(table_count/2);
        else
            i = table_index-(table_count/2);
    }
    int radius = 8*table_count;
    if(radius < 100) radius = 100;
    qreal dtheta = 2*M_PI*i/table_count;
    if(xs < 0) {
        setPos((-radius*cos(atan(ys/xs))+radius*(dtheta)*sin(atan(ys/xs))),
                     (-radius*sin(atan(ys/xs))-radius*(dtheta)*cos(atan(ys/xs))));
    }
    else if(xs > 0) {
        setPos(radius*cos(atan(ys/xs))-radius*(dtheta)*sin(atan(ys/xs)),
                     radius*sin(atan(ys/xs))+radius*(dtheta)*cos(atan(ys/xs)));
    }
}

void Table::verticalPosition2()
{
    setPos(0,50*(table_index+1));
}

void Table::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    setColumnData();
    QMenu menu;
    if(column_list.isEmpty())
        menu.addAction(QIcon(":/icons/design.svg"), tr("Designer"));
    else {
        menu.addAction(QIcon(":/icons/table.svg"), tr("Contents"));
        menu.addAction(tr("Definition"));
        menu.addSeparator();
        menu.addAction(tr("Rename"));
        menu.addSeparator();
        menu.addAction(tr("Clear"));
    }
    menu.addSeparator();
    menu.addAction(tr("Delete"));

    QAction *a = menu.exec(event->screenPos());
    if(a && QString::compare(a->text(),tr("Contents")) == 0) {
        emit expandTable(database, parent_schema, this);
    }
    else if(a && QString::compare(a->text(),tr("Designer")) == 0) {
        emit designTable(database, parent_schema, this);
    }
    else if(a && QString::compare(a->text(),tr("Definition")) == 0) {
        emit expandTableDefinition(parent_schema, this);
    }
    else if(a && QString::compare(a->text(),tr("Clear")) == 0) {
        emit clearTable(database, parent_schema, this);
    }
    else if(a && QString::compare(a->text(),tr("Rename")) == 0) {
        emit rename(database, parent_schema, this);
    }
    else if(a && QString::compare(a->text(),tr("Delete")) == 0) {
        emit dropTable(database, parent_schema, this);
    }
}

void Table::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
{
    setColumnData();
    if(column_list.isEmpty())
        emit designTable(database, parent_schema, this);
    else
        emit expandTable(database, parent_schema, this);
}

void Table::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    if(searched)
        setToolTip(parent_schema->getName() + "." + getName());
    else
        toolTip().clear();
}

void Table::getSearchTerm(QString search_term)
{
    if(table_name.contains(search_term))
        setSearched(true);
    else
        setSearched(false);
    update();
}

bool Table::advance()
{
    if (newPos == pos())
        return false;
    setPos(newPos);
    return true;
}

/*
QVariant Table::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemPositionHasChanged:
        foreach (TableLink *edge, edgeList)
            edge->adjust();
        break;
    default:
        break;
    };
    return QGraphicsItem::itemChange(change, value);
}

void Table::addEdge(TableLink *edge)
{
    edgeList << edge;
    edge->adjust();
}

QList<TableLink *> Table::edges() const
{
    return edgeList;
}
*/
