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
#include "view.h"
#include "schema.h"

View::View(Database *database, Schema *schema, QString view_name, int view_index, QColor color)
{
    this->database = database;
    setParent(schema);
    setParentItem(schema);
    this->view_index = view_index;
    setName(view_name);
    ascii_length = view_name.toAscii().length();
    utf8_length = view_name.toUtf8().length();
    setStatus(false);
    setCollapsed(true);
    //setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    //setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-10);
    setAcceptHoverEvents(true);
}

void View::defaultPosition()
{
    float xs = parent_schema->x();
    float ys = parent_schema->y();
    float i;
    int view_count = parent_schema->getViewCount();
    if(view_count%2 == 0) {
        if (xs < 0)
            i = -view_index+(view_count/2)-0.5;
        else
            i = view_index-(view_count/2)+0.5;
    }
    else {
        if (xs < 0)
            i = -view_index+(view_count/2);
        else
            i = view_index-(view_count/2);
    }
    int radius = 8*view_count;
    if(radius < 100) radius = 100;
    qreal dtheta = 2*M_PI*i/view_count;
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

void View::verticalPosition()
{
    float xs = parent_schema->x();
    float ys = 0;
    float i;
    int view_count = parent_schema->getViewCount();
    if(view_count%2 == 0) {
        if (xs < 0)
            i = -view_index+(view_count/2)-0.5;
        else
            i = view_index-(view_count/2)+0.5;
    }
    else {
        if (xs < 0)
            i = -view_index+(view_count/2);
        else
            i = view_index-(view_count/2);
    }
    int radius = 8*view_count;
    if(radius < 100) radius = 100;
    qreal dtheta = 2*M_PI*i/view_count;
    if(xs < 0)
        setPos((-radius*cos(atan(ys/xs))+radius*(dtheta)*sin(atan(ys/xs))),
                     (-radius*sin(atan(ys/xs))-radius*(dtheta)*cos(atan(ys/xs))));
    else if(xs > 0)
        setPos(radius*cos(atan(ys/xs))-radius*(dtheta)*sin(atan(ys/xs)),
                     radius*sin(atan(ys/xs))+radius*(dtheta)*cos(atan(ys/xs)));
}

void View::verticalPosition2()
{
    setPos(0,50*(view_index+1));
}

void View::setColumnData()
{
    QSqlQuery column_query(database->getDatabaseConnection());
    QString column_query_string = "SELECT a.attname, pg_catalog.format_type(a.atttypid, a.atttypmod), a.atttypmod-4 FROM pg_catalog.pg_attribute a WHERE a.attrelid in (SELECT c.oid FROM pg_catalog.pg_class c LEFT JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace WHERE n.nspname='" + parent_schema->getName() + "' and c.relname='" + view_name + "') AND a.attnum > 0 AND NOT a.attisdropped ORDER BY a.attnum";
    column_query.exec(column_query_string);
    if(column_query.lastError().isValid())
    {
        QMessageBox *error_message = new QMessageBox(QMessageBox::Critical,
                                    tr("Database error"),
                                    tr("Unable to retrieve schema views.\n"
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

void View::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    menu.addAction(QIcon(":/icons/view2.png"), tr("View contents"));
    menu.addAction(tr("View definition"));
    menu.addSeparator();
    menu.addAction(tr("Drop view"));

    QAction *a = menu.exec(event->screenPos());
    if(a && QString::compare(a->text(),tr("View contents")) == 0) {
        emit expandView(database, parent_schema, this);
    }
    else if(a && QString::compare(a->text(),tr("View definition")) == 0) {
        emit expandViewDefinition(parent_schema, this);
    }
    else if(a && QString::compare(a->text(),tr("Drop view")) == 0) {
        emit dropView(database, parent_schema, this);
    }
}

void View::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
{
    emit expandView(database, parent_schema, this);
}

void View::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    if(searched)
        setToolTip(parent_schema->getName() + "." + getName());
    else
        toolTip().clear();
}

void View::getSearchTerm(QString search_term)
{
    if(view_name.contains(search_term))
        setSearched(true);
    else
        setSearched(false);
    update();
}

bool View::advance()
{
    if (newPos == pos())
        return false;
    setPos(newPos);
    return true;
}
