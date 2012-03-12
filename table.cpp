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

#include "mainwin.h"
#include "database.h"
#include "table.h"
#include "schema.h"

Table::Table(Database *database, Schema *schema, QString table_name, int table_index, QColor color)
{
    this->database = database;
    this->setParent(schema);
    this->setParentItem(schema);
    this->table_index = table_index;
    this->setName(table_name);
    ascii_length = table_name.toAscii().length();
    utf8_length = table_name.toUtf8().length();
    this->setStatus(false);
    this->setCollapsed(true);
    createBrush();
    //setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    //setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-10);
    setAcceptHoverEvents(true);
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
        this->setPos((-radius*cos(atan(ys/xs))+radius*(dtheta)*sin(atan(ys/xs))),
                     (-radius*sin(atan(ys/xs))-radius*(dtheta)*cos(atan(ys/xs))));
    }
    else if(xs > 0)
    {
        this->setPos(radius*cos(atan(ys/xs))-radius*(dtheta)*sin(atan(ys/xs)),
                     radius*sin(atan(ys/xs))+radius*(dtheta)*cos(atan(ys/xs)));
    }
}

void Table::setColumnData()
{
    QSqlQuery column_query(database->getDatabaseConnection());
    QString column_query_string = "SELECT column_name, data_type, character_maximum_length FROM information_schema.columns WHERE table_schema='" + parent_schema->getName() + "' AND table_name='" + table_name + "'";
    column_query.exec(column_query_string);
    if(column_query.lastError().isValid())
    {
        QMessageBox *error_message = new QMessageBox(QMessageBox::Critical,
                                    tr("Database error"),
                                    tr("Unable to retrieve schema tables.\n"
                                    "Check your database connection or permissions.\n"), QMessageBox::Cancel,0,Qt::Dialog);
        error_message->setWindowModality(Qt::NonModal);
        error_message->show();
        return;
    }
    //Clear the column list just before populating it.
    column_list.clear();
    column_types.clear();
    column_lengths.clear();
    while (column_query.next()) {
        column_list.append("\"" + column_query.value(0).toString() + "\"");
        column_types.append(column_query.value(1).toString());
        column_lengths.append(column_query.value(2).toString());
    }
}

void Table::copyPrimaryKey()
{
    QSqlQuery column_query(database->getDatabaseConnection());
    QString column_query_string = "SELECT column_name, constraint_name FROM information_schema.constraint_column_usage WHERE table_schema='" + parent_schema->getName() + "' AND table_name='" + table_name + "'";
    column_query.exec(column_query_string);
    if(column_query.lastError().isValid())
    {
        QMessageBox *error_message = new QMessageBox(QMessageBox::Critical,
                                    tr("Database error"),
                                    tr("Unable to retrieve schema tables.\n"
                                    "Check your database connection or permissions.\n"), QMessageBox::Cancel,0,Qt::Dialog);
        error_message->setWindowModality(Qt::NonModal);
        error_message->show();
        return;
    }
    primary_key.clear();
    while (column_query.next())
        primary_key.append("\"" + column_query.value(0).toString() + "\"");
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
    if(xs < 0)
    {
        this->setPos((-radius*cos(atan(ys/xs))+radius*(dtheta)*sin(atan(ys/xs))),
                     (-radius*sin(atan(ys/xs))-radius*(dtheta)*cos(atan(ys/xs))));
    }
    else if(xs > 0)
    {
        this->setPos(radius*cos(atan(ys/xs))-radius*(dtheta)*sin(atan(ys/xs)),
                     radius*sin(atan(ys/xs))+radius*(dtheta)*cos(atan(ys/xs)));
    }
}

void Table::verticalPosition2()
{
    setPos(0,50*(table_index+1));
}

void Table::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    //menu.setStyleSheet("QMenu { font-size:12px; width: 100px; color:white; left: 20px; background-color:qlineargradient(x1:0, y1:0, x2:0, y2:1, stop: 0 #cccccc, stop: 1 #333333);}");
    menu.addAction(QIcon(qApp->applicationDirPath().append("/icons/table.png")), tr("View contents"));
    if(!is_view) {
        menu.addSeparator();
        menu.addAction(tr("Clear contents"));
        menu.addSeparator();
        menu.addAction(tr("Drop table"));
    }
    QAction *a = menu.exec(event->screenPos());
    if(a && QString::compare(a->text(),tr("View contents")) == 0)
    {
        emit expandTable(database, parent_schema, this);
    }
    else if(a && QString::compare(a->text(),tr("Clear contents")) == 0)
    {
        emit clearTable(database, parent_schema, this);
    }
    else if(a && QString::compare(a->text(),tr("Drop table")) == 0)
    {
        emit dropTable(database, parent_schema, this);
    }
}

void Table::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
{
    /*
    if(this->getCollapsed())
    {
        this->setCollapsed(false);
        this->view->show();;
    }
    else
    {
        this->setCollapsed(true);
        this->view->raise();
    }*/
    /*
    if(this->getCollapsed())
    {
        this->setCollapsed(false);
        emit expand(this);
    }
    else
    {
        this->setCollapsed(true);
        emit collapse(this);
    }*/
    emit expandTable(database, parent_schema, this);
    //update();
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
